#!/usr/bin/python2
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_deqp_stats.py:
#   Checks output of deqp testers and generates stats using the GDocs API
#
# prerequirements:
#   https://devsite.googleplex.com/sheets/api/quickstart/python
#   Follow the quickstart guide.
#
# usage: generate_deqp_stats.py [-h] [auth_path] [spreadsheet]
#
# positional arguments:
#   auth_path    path to directory containing authorization data.
#                (credentials.json and token.pickle)
#   spreadsheet  ID of the spreadsheet to write stats to.
#
# optional arguments:
#   -h, --help   show this help message and exit

import re
import datetime
import os
import subprocess
import sys
import argparse
import pickle
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
import urllib

####################
# Global Constants #
####################

SCRIPT_DIR = sys.path[0]
ROOT_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..'))

SCOPES = ['https://www.googleapis.com/auth/spreadsheets']

BOT_NAMES = [
    'Win10 FYI dEQP Release (NVIDIA)',
    'Win10 FYI dEQP Release (Intel HD 630)',
    'Win7 FYI dEQP Release (AMD)',
    'Win7 FYI x64 dEQP Release (NVIDIA)',
    'Mac FYI dEQP Release Intel',
    'Mac FYI dEQP Release AMD',
    'Linux FYI dEQP Release (Intel HD 630)',
    'Linux FYI dEQP Release (NVIDIA)',
    'Android FYI dEQP Release (Nexus 5X)',
    'Android FYI 32 dEQP Vk Release (Pixel XL)',
    'Android FYI 64 dEQP Vk Release (Pixel XL)',
    'Android FYI 32 dEQP Vk Release (Pixel 2)',
    'Android FYI 64 dEQP Vk Release (Pixel 2)',
]
BOT_NAME_PREFIX = 'chromium/ci/'
BUILD_LINK_PREFIX = 'https://ci.chromium.org/p/chromium/builders/ci/'

REQUIRED_COLUMNS = ['build_link', 'time', 'date', 'revision', 'angle_revision']

INFO_TAG = '*RESULT'

######################
# Build Info Parsing #
######################


# Returns a struct with info about the latest successful build given a bot name
# Info contains the build_name, time, date, angle_revision, and chrome revision
# Uses: bb ls '<botname>' -n 1 -status success -A
def get_latest_success_build_info(bot_name):
  bb = subprocess.Popen(
      ['bb', 'ls', bot_name, '-n', '1', '-status', 'success', '-A'],
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE)
  print("Parsing output of [bb ls '" + bot_name + "'-n 1 -status success -A]")
  out, err = bb.communicate()
  if err:
    raise ValueError("Unexpected error from bb ls: '" + err + "'")
  if not out:
    raise ValueError("Unexpected empty result from bb ls of bot '" + bot_name +
                     "'")
  if 'SUCCESS' not in out:
    raise ValueError("Unexpected result from bb ls: '" + out + "'")
  info = {}
  for line in out.splitlines():
    # The first line holds the build name
    if 'build_name' not in info:
      info['build_name'] = line.strip().split("'")[1]
      info['build_link'] = BUILD_LINK_PREFIX + urllib.quote(
          info['build_name'].split(BOT_NAME_PREFIX)[1])
    if 'Created' in line:
      info['time'] = re.findall(r'[0-9]{1,2}:[0-9]{2}:[0-9]{2}',
                                line.split(',', 1)[0])[0]
      info['date'] = datetime.datetime.now().strftime('%y/%m/%d')
    if 'got_angle_revision' in line:
      info['angle_revision'] = filter(str.isalnum, line.split(':')[1])
    if '"revision"' in line:
      info['revision'] = filter(str.isalnum, line.split(':')[1])
  if 'build_name' not in info:
    raise ValueError("Could not find build_name from bot '" + bot_name + "'")
  return info


# Returns a list of step names that we're interested in given a build name. We
# are interested in step names starting with 'angle_'.
# Uses: bb get '<build_name>' -steps
# May raise an exception.
def get_step_names(build_name):
  bb = subprocess.Popen(['bb', 'get', build_name, '-steps'],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
  print("Parsing output of [bb get '" + build_name + "' -steps]")
  out, err = bb.communicate()
  if err:
    raise ValueError("Unexpected error from bb get: '" + err + "'")
  step_names = []
  for line in out.splitlines():
    if 'Step "angle_' not in line:
      continue
    step_names.append(line.split('"')[1])
  return step_names


# Performs some heuristic validation of the step_info struct returned from a
# single step log. Returns True if valid, False if invalid. May write to stderr
def validate_step_info(step_info, build_name, step_name):
  print_name = "'" + build_name + "': '" + step_name + "'"
  if not step_info:
    sys.stderr.write('WARNING: Step info empty for ' + print_name + '\n')
    return False

  if 'Total' in step_info:
    partial_sum_keys = [
        'Passed', 'Failed', 'Skipped', 'Not Supported', 'Exception', 'Crashed'
    ]
    partial_sum_values = [
        int(step_info[key]) for key in partial_sum_keys if key in step_info
    ]
    computed_total = sum(partial_sum_values)
    if step_info['Total'] != computed_total:
      sys.stderr.write('WARNING: Step info does not sum to total for ' +
                       print_name + ' | Total: ' + str(step_info['Total']) +
                       ' - Computed total: ' + str(computed_total) + '\n')
  return True


# Returns a struct containing parsed info from a given step log. The info is
# parsed by looking for lines with the following format in stdout:
# '[TESTSTATS]: <key>: <value>''
# May write to stderr
# Uses: bb log '<build_name>' '<step_name>'
def get_step_info(build_name, step_name):
  bb = subprocess.Popen(['bb', 'log', build_name, step_name],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
  print("Parsing output of [bb log '" + build_name + "' '" + step_name + "']")
  out, err = bb.communicate()
  if err:
    sys.stderr.write("WARNING: Unexpected error from bb log '" + build_name +
                     "' '" + step_name + "': '" + err + "'")
    return None
  step_info = {}
  for line in out.splitlines():
    if INFO_TAG not in line:
      continue
    found_stat = True
    line_columns = line.split(INFO_TAG, 1)[1].split(':')
    if len(line_columns) is not 3:
      sys.stderr.write("WARNING: Line improperly formatted: '" + line + "'\n")
      continue
    key = line_columns[1].strip()
    if key not in step_info:
      step_info[key] = 0
    val = int(filter(str.isdigit, line_columns[2]))
    if val is not None:
      step_info[key] += val
    else:
      step_info[key] += ', ' + line_columns[2]
  if validate_step_info(step_info, build_name, step_name):
    return step_info
  return None


# Returns the info for each step run on a given bot_name.
def get_bot_info(bot_name):
  #print("Parsing bot '" + bot_name + "'")
  info = get_latest_success_build_info(bot_name)
  info['step_names'] = get_step_names(info['build_name'])
  for step_name in info['step_names']:
    #print("  Step '" + step_name + "'")
    info[step_name] = get_step_info(info['build_name'], step_name)
  return info


#####################
# Sheets Formatting #
#####################


# Get an individual spreadsheet based on the spreadsheet id. Returns the result
# of spreadsheets.get(), or throws an exception if the sheet could not open.
def get_spreadsheet(service, spreadsheet_id):
  print("Opening spreadsheet [spreadsheets.get(spreadsheetId='" +
        spreadsheet_id + "')]")
  request = service.get(spreadsheetId=spreadsheet_id)
  spreadsheet = request.execute()
  if not spreadsheet:
    raise Exception("Did not open spreadsheet '" + spreadsheet_id + "'")
  return spreadsheet


# Returns a nicely formatted string based on the bot_name and step_name
def format_sheet_name(bot_name, step_name):
  unneccesary_tokens = ['FYI', 'Release', 'Vk', 'dEQP', '(', ')']
  for token in unneccesary_tokens:
    bot_name = bot_name.replace(token, '')
  bot_name = ' '.join(bot_name.strip().split())  # Remove extra spaces
  step_name = re.findall(r'angle\w*', step_name)[0]
  return bot_name + ' ' + step_name


# Returns the full list of sheet names that should be populated based on the
# info struct
def get_sheet_names(info):
  sheet_names = []
  for bot_name in info:
    for step_name in info[bot_name]['step_names']:
      sheet_name = format_sheet_name(bot_name, step_name)
      sheet_names.append(sheet_name)
  return sheet_names


# Returns True if the sheet is found in the spreadsheets object
def sheet_exists(spreadsheet, step_name):
  for sheet in spreadsheet['sheets']:
    if sheet['properties']['title'] == step_name:
      return True
  return False


# Validates the spreadsheets object against the list of sheet names which
# should appear. Returns a list of sheets that need creation.
def validate_sheets(spreadsheet, sheet_names):
  create_sheets = []
  for sheet_name in sheet_names:
    if not sheet_exists(spreadsheet, sheet_name):
      create_sheets.append(sheet_name)
  return create_sheets


# Performs a batch update with a given service, spreadsheet id, and list of 
# updates <object(Request)> to do.
def batch_update(service, spreadsheet_id, updates):
  batch_update_request_body = {
      'requests': updates,
  }
  print("Creating new sheets [spreadsheets.batchUpdate(spreadsheetId='" +
        spreadsheet_id + "', body=" + str(batch_update_request_body) + ')]')
  request = service.batchUpdate(
      spreadsheetId=spreadsheet_id, body=batch_update_request_body)
  request.execute()


# Creates sheets given a service and spreadsheed id based on a list of sheet
# names input
def create_sheets(service, spreadsheet_id, sheet_names):
  updates = [{
      'addSheet': {
          'properties': {
              'title': sheet_name,
          }
      }
  } for sheet_name in sheet_names]
  batch_update(service, spreadsheet_id, updates)


# Calls a values().batchGet() on the service to find the list of column names
# from each sheet in sheet_names. Returns a dictionary with one list per
# sheet_name.
def get_headers(service, spreadsheet_id, sheet_names):
  header_ranges = [sheet_name + '!A1:Z' for sheet_name in sheet_names]
  print("Getting headers [spreadsheets.values().batchGet(spreadsheetId='" +
        spreadsheet_id + ', ranges=' + str(header_ranges) + "')]")
  request = service.values().batchGet(
      spreadsheetId=spreadsheet_id, ranges=header_ranges)
  response = request.execute()
  headers = {}
  for k, sheet_name in enumerate(sheet_names):
    if 'values' in response['valueRanges'][k]:
      headers[sheet_name] = response['valueRanges'][k]['values'][0]
    else:
      headers[sheet_name] = []
  return headers


# Calls values().batchUpdate() with supplied list of data <object(ValueRange)>
# to update on the service.
def batch_update_values(service, spreadsheet_id, data):
  batch_update_values_request_body = {
      'valueInputOption': 'RAW',
      'data': data,
  }
  print("Updating headers [spreadsheets.values().batchUpdate(spreadsheetId='" +
        spreadsheet_id + "', body=" + str(batch_update_values_request_body) +
        ')]')
  request = service.values().batchUpdate(
      spreadsheetId=spreadsheet_id, body=batch_update_values_request_body)
  request.execute()


# Populates the headers with any missing/desired rows based on the info struct,
# and calls batch update to update the corresponding sheets if necessary.
def update_headers(service, spreadsheet_id, headers, info):
  data = []
  sheet_names = []
  for bot_name in info:
    for step_name in info[bot_name]['step_names']:
      sheet_name = format_sheet_name(bot_name, step_name)
      headers_stale = False
      # Headers should always contain the following columns
      for req in REQUIRED_COLUMNS:
        if req not in headers[sheet_name]:
          headers_stale = True
          headers[sheet_name].append(req)
      # Headers also must contain all the keys seen in this step
      for key in info[bot_name][step_name].keys():
        if key not in headers[sheet_name]:
          headers_stale = True
          headers[sheet_name].append(key)
      # Update the Gdoc headers if necessary
      if headers_stale:
        sheet_names.append(sheet_name)
        header_range = sheet_name + '!A1:Z'
        data.append({
            'range': header_range,
            'majorDimension': 'ROWS',
            'values': [headers[sheet_name]]
        })
  if data:
    batch_update_values(service, spreadsheet_id, data)


# Calls values().append() to append a list of values to a given sheet.
def append_values(service, spreadsheet_id, sheet_name, values):
  header_range = sheet_name + '!A1:Z'
  insert_data_option = 'INSERT_ROWS'
  value_input_option = 'RAW'
  append_values_request_body = {
      'range': header_range,
      'majorDimension': 'ROWS',
      'values': [values],
  }
  print("Appending values [spreadsheets.values().append(spreadsheetId='" +
        spreadsheet_id + "', body=" + str(append_values_request_body) +
        ", range='" + header_range + "', insertDataOption='" +
        insert_data_option + "', valueInputOption='" + value_input_option +
        "')]")
  request = service.values().append(
      spreadsheetId=spreadsheet_id,
      body=append_values_request_body,
      range=header_range,
      insertDataOption=insert_data_option,
      valueInputOption=value_input_option)
  request.execute()


# Uses the list of headers and the info struct to come up with a list of values
# for each step from the latest builds.
def update_values(service, spreadsheet_id, headers, info):
  data = []
  for bot_name in info:
    for step_name in info[bot_name]['step_names']:
      sheet_name = format_sheet_name(bot_name, step_name)
      values = []
      for key in headers[sheet_name]:
        if key in info[bot_name] and key in REQUIRED_COLUMNS:
          values.append(info[bot_name][key])
        elif key in info[bot_name][step_name]:
          values.append(info[bot_name][step_name][key])
        else:
          values.append('')
      append_values(service, spreadsheet_id, sheet_name, values)


# Updates the given spreadsheed_id with the info struct passed in.
def update_spreadsheet(service, spreadsheet_id, info):
  spreadsheet = get_spreadsheet(service, spreadsheet_id)
  sheet_names = get_sheet_names(info)
  new_sheets = validate_sheets(spreadsheet, sheet_names)
  if new_sheets:
    create_sheets(service, spreadsheet_id, new_sheets)
  headers = get_headers(service, spreadsheet_id, sheet_names)
  update_headers(service, spreadsheet_id, headers, info)
  update_values(service, spreadsheet_id, headers, info)


#####################
# Main/helpers      #
#####################


# Loads or creates credentials and connects to the Sheets API. Returns a
# Spreadsheets object with an open connection.
def get_sheets_service(auth_path):
  credentials_path = auth_path + '/credentials.json'
  token_path = auth_path + '/token.pickle'
  creds = None
  if not os.path.exists(auth_path):
    print("Creating auth dir '" + auth_path + "'")
    os.makedirs(auth_path)
  if not os.path.exists(credentials_path):
    raise Exception(
        'Missing credentials.json.\n'
        'Go to: https://developers.google.com/sheets/api/quickstart/python\n'
        "Under Step 1, click 'ENABLE THE GOOGLE SHEETS API'\n"
        "Click 'DOWNLOAD CLIENT CONFIGURATION'\n"
        'Save to your auth_path (' + auth_path + ') as credentials.json')
  if os.path.exists(token_path):
    with open(token_path, 'rb') as token:
      creds = pickle.load(token)
      print('Loaded credentials from ' + token_path)
  if not creds or not creds.valid:
    if creds and creds.expired and creds.refresh_token:
      print('Refreshing credentials...')
      creds.refresh(Request())
    else:
      print('Could not find credentials. Requesting new credentials.')
      flow = InstalledAppFlow.from_client_secrets_file(
        credentials_path, SCOPES)
      creds = flow.run_local_server()
    with open(token_path, 'wb') as token:
      pickle.dump(creds, token)
  service = build('sheets', 'v4', credentials=creds)
  sheets = service.spreadsheets()
  return sheets


# Parse the input to the script
def parse_args():
  parser = argparse.ArgumentParser(os.path.basename(sys.argv[0]))
  parser.add_argument(
      'auth_path',
      default=ROOT_DIR + '/.auth',
      nargs='?',
      help='path to directory containing authorization data. '
           '(credentials.json and token.pickle)'
  )
  parser.add_argument(
      'spreadsheet',
      default='1D6Yh7dAPP-aYLbX3HHQD8WubJV9XPuxvkKowmn2qhIw',
      nargs='?',
      help='ID of the spreadsheet to write stats to.')
  return parser.parse_args()


def main():
  args = parse_args()
  auth_path = args.auth_path.replace('\\', '/')
  try:
    service = get_sheets_service(auth_path)
  except Exception as error:
    sys.stderr.write('ERROR: %s\n' % str(error))
    exit(1)

  info = {}
  print('Building info struct...')
  for bot_name in BOT_NAMES:
    try:
      info[bot_name] = get_bot_info(BOT_NAME_PREFIX + bot_name)
    except Exception as error:
      sys.stderr.write('ERROR: %s\n' % str(error))

  print('Updating sheets...')
  try:
    update_spreadsheet(service, args.spreadsheet, info)
  except Exception as error:
    sys.stderr.write('ERROR: %s\n' % str(error))
    quit(1)

  print(
      'Info was successfully parsed to sheet: '
      'https://docs.google.com/spreadsheets/d/'
      + args.spreadsheet)


if __name__ == '__main__':
  sys.exit(main())
