import sys
import os
import gzip
import json

trace_path = os.path.join('..', '..', 'src', 'tests', 'restricted_traces', sys.argv[1])

jpath = os.path.join(trace_path, sys.argv[1] + '.json')
with open(jpath, 'rb') as f:
    jcont = f.read()

j = json.loads(jcont)

with open('gen/bundle_' + sys.argv[1] + '.gz', 'wb') as f:
    with gzip.GzipFile(fileobj=f, mode='wb', compresslevel=9, mtime=0) as fgzip:
        for fn in j['TraceFiles']:
            if fn.endswith('.cpp'):
                with open(os.path.join(trace_path, fn), 'rb') as fcpp:
                    fgzip.write(fcpp.read())
