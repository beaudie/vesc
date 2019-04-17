#!/usr/bin/python2
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_deqp_stats.py:
#   Checks output of deqp testers and generates stats using the GDocs API

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

INFO_TAG = '*RESULT'


# Returns a struct with info about the latest successful build given a bot name
# Info contains the build_name, time, date, angle_revision, and chrome revision
# Uses: bb ls '<botname>' -n 1 -status success -A
def get_latest_success_build_info(bot_name):
  bb = subprocess.Popen(
      ['bb', 'ls', bot_name, '-n', '1', '-status', 'success', '-A'],
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE)
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
    if 'Created' in line:
      info['time'] = re.findall(r'[0-9]{1,2}:[0-9]{2}:[0-9]{2}',
                                line.split(',', 1)[0])[0]
      info['date'] = datetime.datetime.now().strftime('%y/%m/%d')
    if 'parent_got_angle_revision' in line:
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
  info = get_latest_success_build_info(bot_name)
  info['step_names'] = get_step_names(info['build_name'])
  for step_name in info['step_names']:
    info[step_name] = get_step_info(info['build_name'], step_name)
  return info


def get_sheets_service(auth_path):
  credentials_path = auth_path + '/credentials.json'
  token_path = auth_path + '/token.pickle'
  creds = None
  if os.path.exists(token_path):
    with open(token_path, 'rb') as token:
      creds = pickle.load(token)
  if not creds or not creds.valid:
    if creds and creds.expired and creds.refresh_token:
      creds.refresh(Request())
    else:
      flow = InstalledAppFlow.from_client_secrets_file(credentials_path, SCOPES)
      creds = flow.run_local_server()
    with open(token_path, 'wb') as token:
      pickle.dump(creds, token)
  service = build('sheets', 'v4', credentials=creds)
  sheets = service.spreadsheets()
  return sheets


def get_spreadsheet(service, spreadsheet_id):
  request = service.get(spreadsheetId=spreadsheet_id)
  return request.execute()


def sheet_exists(spreadsheet, step_name):
  for sheet in spreadsheet['sheets']:
    if sheet['properties']['title'] == step_name:
      return True
  return False


def create_sheet(service, spreadsheet_id, step_name):
  batch_update_values_request_body = {
      'requests': [{
          'addSheet': {
              'properties': {
                  'title': step_name,
              }
          }
      }]
  }
  request = service.batchUpdate(
      spreadsheetId=spreadsheet_id, body=batch_update_values_request_body)
  request.execute()


def update_headers(service, spreadsheet_id, step_name, headers):
  header_range = step_name + '!A1:Z'
  batch_update_values_request_body = {
      'valueInputOption': 'RAW',
      'includeValuesInResponse': True,
      'data': [{
          'range': header_range,
          'values': [headers]
      }],
  }
  request = service.values().batchUpdate(
      spreadsheetId=spreadsheet_id, body=batch_update_values_request_body)
  return request.execute()['responses'][0]['updatedData']['values'][0]


def get_headers(service, spreadsheet_id, step_name, keys):
  header_range = step_name + '!A1:Z'
  request = service.values().get(
      spreadsheetId=spreadsheet_id, range=header_range)
  response = request.execute()
  headers = []
  if 'values' in response:
    headers = request.execute()['values'][0]
  headers_stale = False
  if 'build_name' not in headers:
    headers_stale = True
    headers.append('build_name')
  if 'time' not in headers:
    headers_stale = True
    headers.append('time')
  if 'date' not in headers:
    headers_stale = True
    headers.append('date')
  if 'revision' not in headers:
    headers_stale = True
    headers.append('revision')
  if 'angle_revision' not in headers:
    headers_stale = True
    headers.append('angle_revision')
  for key in keys:
    if key not in headers:
      headers_stale = True
      headers.append(key)
  if headers_stale:
    headers = update_headers(service, spreadsheet_id, step_name, headers)
  return headers


def get_values(info, headers, bot_name, step_name):
  values = []
  for key in headers:
    if key in info[bot_name]:
      values.append(info[bot_name][key])
    elif key in info[bot_name][step_name]:
      values.append(info[bot_name][step_name][key])
    else:
      values.append('')
  return values


def append_values(service, spreadsheet_id, step_name, values):
  header_range = step_name + '!A1:Z'
  insert_data_option = 'INSERT_ROWS'
  value_input_option = 'RAW'
  batch_append_values_request_body = {
      'range': header_range,
      'majorDimension': 'ROWS',
      'values': [values,],
  }
  request = service.values().append(
      spreadsheetId=spreadsheet_id,
      body=batch_append_values_request_body,
      range=header_range,
      insertDataOption=insert_data_option,
      valueInputOption=value_input_option)
  request.execute()


def update_sheets(service, spreadsheet_id, info):
  spreadsheet = get_spreadsheet(service, spreadsheet_id)
  for bot_name in info:
    for step_name in info[bot_name]['step_names']:
      if not sheet_exists(spreadsheet, step_name):
        create_sheet(service, spreadsheet_id, step_name)
        spreadsheet = get_spreadsheet(service, spreadsheet_id)
      headers = get_headers(service, spreadsheet_id, step_name,
                            info[bot_name][step_name].keys())
      values = get_values(info, headers, bot_name, step_name)
      append_values(service, spreadsheet_id, step_name, values)


def test(service, spreadsheet):
  spreadsheet_id = spreadsheet  # TODO: Update placeholder value.

  batch_update_values_request_body = {
      # How the input data should be interpreted.
      'value_input_option': 'RAW',  # TODO: Update placeholder value.

      # The new values to apply to the spreadsheet.
      'data': [{
          'range': 'Sheet2!A1:Z',
          'values': [[1, 2, 3]]
      }],  # TODO: Update placeholder value.

      # TODO: Add desired entries to the request body.
  }

  request = service.values().batchUpdate(
      spreadsheetId=spreadsheet_id, body=batch_update_values_request_body)
  return request.execute()


def parse_args():
  parser = argparse.ArgumentParser(os.path.basename(sys.argv[0]))
  parser.add_argument(
      'auth_path',
      default='~/.angle_auth',
      help='path to directory containing authorization data. (credentials.json and token.pickle)'
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
  service = get_sheets_service(auth_path)
  if not service:
    raise Exception('Unable to connect to Sheets API')
  info = {}
  for bot_name in BOT_NAMES:
    try:
      info[bot_name] = get_bot_info(BOT_NAME_PREFIX + bot_name)
    except Exception as error:
      sys.stderr.write('ERROR: %s\n' % str(error))
  update_sheets(service, args.spreadsheet, info)


if __name__ == '__main__':
  sys.exit(main())
