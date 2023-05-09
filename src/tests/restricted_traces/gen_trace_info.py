#!/usr/bin/python3

import os
import sys
import json

inputs_file, output_json_file = sys.argv[1:]

with open(inputs_file) as f:
    inputs = f.read().split(' ')

all_trace_infos = []
for fn in inputs:
    with open(fn) as f:
        trace = os.path.splitext(os.path.basename(fn))[0]
        trace_info = {'TraceName': trace}
        trace_info.update(json.load(f))
        all_trace_infos.append(trace_info)

with open(output_json_file, 'w') as f:
    json.dump(all_trace_infos, f, indent=2)
