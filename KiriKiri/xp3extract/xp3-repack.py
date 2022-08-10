#!/usr/bin/env python

# KiriKiri .XP3 archive repacking tool
#
#   Packs a directory of files into an .XP3 archive, including any
# subdirectory structure.
#
#   Optionally handles Fate Stay Night encryption.
#
# Last modified 2006-07-08, Edward Keyes, ed-at-insani-dot-org
# jpnoob in 2021: changed encoding to match my version of extract

import string, sys, os, zlib
from array import array
from cStringIO import StringIO
from insani import *


if len(sys.argv) not in (3,4) :
   print 'Please give a input directory and a desired output XP3 archive filename on\nthe command line.  Append an optional encryption type.'
   sys.exit(0)


def write_entry(outfile, entry) :
# Writes a file entry data structure to the file.
   outfile.write('File')   
   write_unsigned(outfile,len(entry['filepath'])*2+len(entry['segments'])*28 \
                          +62, LONG_LENGTH)
   outfile.write('info')
   write_unsigned(outfile,len(entry['filepath'])*2+22,LONG_LENGTH)
   write_unsigned(outfile,entry['encrypted'])
   write_unsigned(outfile,entry['origsize'],LONG_LENGTH)
   write_unsigned(outfile,entry['compsize'],LONG_LENGTH)
   write_unsigned(outfile,len(entry['filepath']),SHORT_LENGTH)
   for char in entry['filepath'] :
      write_unsigned(outfile,ord(char),SHORT_LENGTH)
   outfile.write('segm')
   write_unsigned(outfile,len(entry['segments'])*28,LONG_LENGTH)
   for segment in entry['segments'] :
      write_unsigned(outfile,segment['compressed'])
      write_unsigned(outfile,segment['offset'],LONG_LENGTH)
      write_unsigned(outfile,segment['origsize'],LONG_LENGTH)
      write_unsigned(outfile,segment['compsize'],LONG_LENGTH)
   outfile.write('adlr')
   write_unsigned(outfile,4,LONG_LENGTH)
   write_unsigned(outfile,entry['adler'])


def encrypt(outfile, encryption) :
# Performs standard types of XP3 encryption on a file.  This is dentical to
# the decrypt function except for the error message since it's all XOR.
   if encryption == 'fate_trial' :
      outfile.seek(0)
      data = array('B',outfile.read())
      for i in xrange(len(data)) :
         data[i] ^= 0x0052
      if len(data)>30 :
         data[30] ^= 0x0001
      if len(data)>55355 :
         data[55355] ^= 0x0020
      outfile.seek(0)
      outfile.write(data.tostring())
   elif encryption == 'fate_full' :   # Not 100% sure about these values.
      outfile.seek(0)
      data = array('B',outfile.read())
      for i in xrange(len(data)) :
         data[i] ^= 0x0036
      if len(data)>19 :
         data[19] ^= 0x0001
      outfile.seek(0)
      outfile.write(data.tostring())
   else :
      print 'WARNING: Unknown encryption type specified, none performed'
      print 'Known types include: fate_trial, fate_full'


dirname=sys.argv[1]
arcfile=open(sys.argv[2],'wb')
if (len(sys.argv)==4) :
   encryption = sys.argv[3]
else :
   encryption = 'none'

# Write header
write_string(arcfile,'XP3\x0D\x0A \x0A\x1A\x8B\x67\x01')
write_unsigned(arcfile,0,LONG_LENGTH)   # Placeholder for index offset

# Scan for files, write them and collect the index as we go
indexbuffer = StringIO()
for (dirpath, dirs, filenames) in os.walk(dirname) :
   assert (dirpath.startswith(dirname))
   newpath = dirpath[len(dirname):]   # Strip off base directory
   if newpath.startswith(os.sep) :    # and possible slash
      newpath = newpath[len(os.sep):]
   pathcomponents = newpath.split(os.sep)
   newpath = string.join(pathcomponents,'/')   # Slashes used inside XP3
   for filename in filenames :
      entry = {}
      segment = {}
      if newpath != '' :
         filepath = newpath + '/' + filename
      else :
         filepath = filename
      entry['filepath'] = unicode(filepath,'utf-8')
      localfilepath = os.path.join(dirpath,filename)
      infile = open(localfilepath,'rb')
      data = infile.read()
      infile.close()
      entry['origsize'] = segment['origsize'] = len(data)
      if (encryption != 'none') :
         entry['encrypted'] = 0x0080000000L
         tempbuffer = StringIO()
         tempbuffer.write(data)
         encrypt(tempbuffer,encryption)
         data = tempbuffer.getvalue()
         tempbuffer.close()
      else :
         entry['encrypted'] = 0
      entry['adler'] = (zlib.adler32(data) + 0x0100000000L) & 0x00FFFFFFFFL
         # Convert to unsigned 32-bit integer
      compressed = zlib.compress(data,9)
      if len(compressed)<0.95*len(data) :  # Don't store compressed if we
         segment['compressed'] = 1         # gain just a few percent from it
         data = compressed
      else :
         segment['compressed'] = 0
      entry['compsize'] = segment['compsize'] = len(data)
      segment['offset'] = arcfile.tell()
      entry['segments'] = [segment]    # Always using a list of one segment
      write_entry(indexbuffer,entry)
      print 'Packing %s (%d -> %d bytes)' % \
         (entry['filepath'].encode('utf-8'),
          entry['origsize'], entry['compsize'])
      arcfile.write(data)

# Now write the index and go back and put its offset in the header
indexoffset = arcfile.tell()
data = indexbuffer.getvalue()
compressed = zlib.compress(data,9)
arcfile.write('\x01')
write_unsigned(arcfile,len(compressed),LONG_LENGTH)
write_unsigned(arcfile,len(data),LONG_LENGTH)
arcfile.write(compressed)
arcfile.seek(11)   # Length of header
write_unsigned(arcfile,indexoffset,LONG_LENGTH)

indexbuffer.close()
arcfile.close()
