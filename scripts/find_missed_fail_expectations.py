# Copyright 2016 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Extracts information about the FAIL dEQP expectations that have been passing
# in the last several test runs, and prints expectations that can be removed.

import requests
import lxml.html.soupparser
import bs4
import string
import os.path

def sanitize_filename(filename):
    accepted_chars = string.ascii_letters + string.digits + "-_."
    filename = filename.replace('/', '_')
    return ''.join(filter(lambda char: char in accepted_chars, filename))

def get_url(url, cached):
    if not cached:
        return requests.get(url).content

    filename = 'cache/' + sanitize_filename(url)

    if not os.path.isfile(filename):
        print("Not in cache:", url)
        content = requests.get(url).content
        with open(filename, 'wb') as f:
            f.write(content)

    with open(filename) as f:
        return f.read()

def get_xml(url, cached):
    def soup(*args, **kwargs):
        if not "html.parser" in args:
            args = list(args) + ["html.parser"]
        return bs4.BeautifulSoup(*args, **kwargs)

    return lxml.html.soupparser.fromstring(get_url(url, cached), beautifulsoup=soup)


base_url = 'https://build.chromium.org/p/chromium.gpu.fyi/builders/'

def get_last_build_id(builder):
    tree = get_xml(base_url + builder, False)
    return int(tree.xpath('(//div[@class = "content"]//table[@class = "info"])[1]//tr[2]//td[4]/a')[0].text[1:])

def get_url_to_swarm_stdio(builder, build_id, test):
    tree = get_xml(base_url + builder + '/builds/' + str(build_id), True)
    relative_url = tree.xpath('((//div[@class = "content"]//ol)[1]//li[contains((.//li)[1]/a/@href, "' + test + '")])[2]/ul/li[1]/a')[0].attrib['href']
    return base_url + builder + '/builds/' + relative_url

def get_missed_expectations(log):
    log = log.split('\n')
    missed = set()
    for i, line in enumerate(log):
        if line.startswith('Test expected to fail but passed!'):
            missed.update((log[i+1][13:].split(' ')[0],))
    return missed

def intersection(sets):
    s = sets[0]
    for r in sets[1:]:
        s = s.intersection(r)
    return s

def get_missed_expectations_for_builder(builder, test):
    build_id = get_last_build_id(builder)
    missed = []
    for i in range(20):
        build = build_id - i
        if build <= 0:
            continue
        stdio = get_url(get_url_to_swarm_stdio(builder, build, test), True)
        missed += [get_missed_expectations(stdio)]

    return intersection(missed)


builders_to_check = [
    ("Windows", [
        'Win7 Release (NVIDIA)',
    ]),
    ('Linux', [
        'Linux Release (NVIDIA)',
    ]),
]

for name, builders in builders_to_check:
    print("******* OS:", name)

    for test in ('angle_deqp_gles2_tests', 'angle_deqp_gles3_tests'):
        expectations = intersection([get_missed_expectations_for_builder(builder, test) for builder in builders])

        print("   MISSED EXPECTATIONS FOR", test)
        for expectation in sorted(list(expectations)):
            print("    ", expectation)
