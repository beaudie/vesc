import sys
import os
import gzip

jpath = os.path.join('..', '..', 'src', 'tests', 'restricted_traces', sys.argv[1], sys.argv[1] + '.json')
with open(jpath, 'rb') as f:
	jcont = f.read()

with open('gen/bundle_' + sys.argv[1] + '.gz', 'wb') as f:
	with gzip.GzipFile(fileobj=f, mode='wb', compresslevel=9, mtime=0) as f:
	    f.write(jcont)
