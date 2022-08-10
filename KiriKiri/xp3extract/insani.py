#!/usr/bin/env python

# insani.py
#
#   Utility routines to support visual-novel game file extraction and
#   modification.
#
# Last modified 2006-06-12, Edward Keyes, ed-at-insani-dot-org

LITTLE_ENDIAN = 1
BIG_ENDIAN = -1

BYTE_LENGTH = 1
SHORT_LENGTH = 2
INT_LENGTH = 4
LONG_LENGTH = 8

ERROR_NONE = 0
ERROR_WARNING = 1
ERROR_ABORT = 2


def read_unsigned(infile, size = INT_LENGTH, endian = LITTLE_ENDIAN) :
# Reads a binary unsigned number from the file of the specified
# length and endianness.  The size isn't restricted to the predefined
# constants.

   result = long(0)
   for i in xrange(size) :
      temp=long(ord(infile.read(1)))
      if endian == LITTLE_ENDIAN :
         result |= (temp << (8*i))
      elif endian == BIG_ENDIAN :
         result = (result << 8) | temp
      else :
         raise 'read_unsigned', 'Unknown endian specification'
   return result


def write_unsigned(outfile, value, size = INT_LENGTH, endian = LITTLE_ENDIAN) :
# Writes a binary unsigned number of the specified length and
# endianness to the file.  Does not check to see if the size is
# sufficient to fully represent the number.  The size isn't restricted
# to the predefined constants.

   for i in xrange(size) :
      if endian == LITTLE_ENDIAN :
         temp = (value >> (8*i)) & 0x00FF
      elif endian == BIG_ENDIAN :
         temp = (value >> (8*(size-i-1))) & 0x00FF
      else :
         raise 'write_unsigned', 'Unknown endian specification'
      outfile.write(chr(temp))


def read_string(infile) :
# Reads a null-terminated string from the given file.  This is not very
# efficient for long strings, but should be okay for reading filenames
# and script lines.  Does not return the final \0, but does keep any line
# feeds and carriage returns encountered.  Also terminates the string at
# EOF.

   result = ''
   temp = infile.read(1)
   while (temp != '') and (temp != '\0') :
      result += temp
      temp = infile.read(1)
   return result


def write_string(outfile, value) :
# Writes a null-terminated string to the given file.  The given string is
# assumed to not have a terminator already.

   outfile.write(value)
   outfile.write('\0')


def escape_string(value) :
# Escapes a string, using hex values of nonprintable characters, for safe
# printing to the console.  Probably a stupid way to do it.

   result = ''
   for temp in value :
      tempval=ord(temp)
      if (tempval<32) or (tempval>126) :
         result += '#%02X' % tempval
      else :
         result += temp
   return result


def unescape_string(value) :
# Unescapes a string encoded with the above routine, replacing hex values
# by real characters.

   result = ''
   i = 0
   while i<len(value) :
      if value[i]=='#' :
         result += chr(int(value[i+1:i+3],16))  # Convert from hex
         i += 3
      else :
         result += value[i]
         i += 1
   return result


def assert_string(infile, value, severity = ERROR_NONE) :
# Checks to see that the file has the given string at the current location.
# Advances the file position past the end of the string.  Returns true if
# assertion succeeds, false otherwise, and optionally prints a warning and
# aborts via raising an exception on failure.

   actual = infile.read(len(value))
   if actual != value :
      if severity != ERROR_NONE :
         print 'Expected "%s" at position 0x%X but saw "%s".' % \
          (escape_string(value),infile.tell()-len(actual),escape_string(actual))
      if severity == ERROR_ABORT :
         print 'Aborting!'
         raise 'assert_string'
      return False
   else :
      return True


def assert_zeroes(infile, num = 1, severity = ERROR_NONE) :
# Checks to see that the file has the specified number of zeroes at
# the current location.  Advances the file position.  Returns true if
# assertion succeeds, false otherwise, and optionally prints a warning
# and aborts via raising an exception.  Only the first instance of an
# error is printed, rather than all of them.

   found_error = -1
   actual=infile.read(num)
   for i in xrange(len(actual)) :
      temp=actual[i]
      if (temp != '\0') :
	 found_error = infile.tell()-len(actual)+i
         break
   if (found_error < 0) and (len(actual)<num) :
      temp = '';
      found_error = infile.tell()
   if found_error >= 0 :
      if severity != ERROR_NONE :
         print 'Expected "\\x00" at position 0x%X but found "%s".' % \
          (found_error,escape_string(temp))
      if severity == ERROR_ABORT :
         print 'Aborting!'
         raise 'assert_zeroes'
      return False
   else :
      return True


def write_zeroes(outfile, num) :
# Just writes the specified number of zero bytes to the file.  Some lame
# optimization for large numbers.

   if num >= 1024 :
      onek = '\0' * 1024
      for i in xrange(num // 1024) :
         outfile.write(onek)
   for i in xrange(num % 1024) :
      outfile.write('\0')



# Perform some unit tests if we're just running this module by itself.

if __name__ == '__main__' :
   from StringIO import StringIO

   teststring_little = '\x01\x02\x03\x04\x00\x00\x00\x00'
   teststring_big = '\x00\x00\x00\x00\x04\x03\x02\x01'
   testvalue = 0x04030201

   testfile = StringIO(teststring_little)
   value = read_unsigned(testfile,LONG_LENGTH)
   assert (value == testvalue)
   testfile.close()

   testfile = StringIO(teststring_big)
   value = read_unsigned(testfile,LONG_LENGTH,BIG_ENDIAN)
   assert (value == testvalue)
   testfile.close()

   testfile = StringIO()
   write_unsigned(testfile,testvalue,LONG_LENGTH)
   assert (teststring_little == testfile.getvalue())
   testfile.close()

   testfile = StringIO()
   write_unsigned(testfile,testvalue,LONG_LENGTH,BIG_ENDIAN)
   assert (teststring_big == testfile.getvalue())
   testfile.close()

   teststring = 'foobar\0'
   testvalue = 'foobar'

   testfile = StringIO(teststring)
   assert (testvalue == read_string(testfile))
   testfile.close()

   testfile = StringIO()
   write_string(testfile,testvalue)
   assert (teststring == testfile.getvalue())
   testfile.close()

   testvalue = '\x01foo\xFE'
   testescape = '#01foo#FE'
   assert (escape_string(testvalue) == testescape)
   assert (unescape_string(testescape) == testvalue)

   teststring = 'foobar\0\0\0\0'
   testvalue = 'foobar'

   testfile = StringIO(teststring)
   assert (assert_string(testfile,testvalue,ERROR_NONE))
   assert (assert_zeroes(testfile,4,ERROR_NONE))
   testfile.close()   

   testfile = StringIO()
   write_zeroes(testfile,4)
   assert ('\0\0\0\0' == testfile.getvalue())
   testfile.close()

   print 'All unit tests completed successfully.'
