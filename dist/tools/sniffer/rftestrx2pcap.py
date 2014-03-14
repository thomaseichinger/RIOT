#!/usr/bin/python2
# (C) 2012, Mariano Alvira <mar@devl.org>
# (C) 2014, Oliver Hahm <oliver.hahm@inria.fr>

import sys,os,time
from struct import *
import re
import serial

if len(sys.argv) < 3:
	sys.stderr.write( "Usage: %s tty channel [outfile]\n" %(sys.argv[0]))
        sys.stderr.write( "       channel = 11-26\n")
	sys.exit(2)

# change the channel

try:
        serport = serial.Serial(sys.argv[1], baudrate=115200, dsrdtr=0, rtscts=0, timeout=1)
        serport.setDTR(0)
        serport.setRTS(0)
except IOError:
	print "error opening port"
	sys.exit(2)

time.sleep(1)
chanstr = ''
sys.stderr.write('chan %s\n' % sys.argv[2])
serport.write('chan %s\n' % sys.argv[2])
while 1:
        c = serport.read(1)
        if (c == '\n'):
            chanstr = ''
            continue
        chanstr += c 
        m = re.match(".*channel: (\w+)", chanstr)
        if m is not None:
                chan = int(m.group(1))
                sys.stderr.write(chanstr + '\n')
                break

try:
	sys.stderr.write('writing to file %s \n' % (sys.argv[3]))
	outfile = open(sys.argv[3], 'w+b')
except IndexError:
	outfile = sys.stdout

sys.stderr.write("RX: 0\r")

### PCAP setup
MAGIC = 0xa1b2c3d4;
MAJOR = 2;
MINOR = 4;
ZONE = 0;
SIG = 0;
SNAPLEN = 0xffff;
NETWORK = 230; # 802.15.4 no FCS

# output overall PCAP header
outfile.write(pack('<LHHLLLL', MAGIC, MAJOR, MINOR, ZONE, SIG, SNAPLEN, NETWORK))

count = 0
fileempty = 1
newpacket = 0

try:
	while 1:
		line = serport.readline().rstrip()

		m_rftestline = re.match(".*rftest-rx --- len 0x(\w\w).*", line)

		if m_rftestline is not None:
			newpacket = 1
			t = time.time()
			sec = int(t)
			usec = (t - sec) * 100000
			length = int(m_rftestline.group(1), 16)
#			sys.stderr.write(line + "\n")
#			sys.stderr.write("rftestline: %d %d %d\n" % (sec, usec, length))
			continue
			
			# if this is a new packet, add a packet header
#                else:
#                    sys.stderr.write("failed to recognize line: %s\n" % line)
		if newpacket == 1:
			newpacket = 0
			outfile.write(pack('<LLLL',sec,usec,length,length))
			outfile.flush()

			count += 1
			sys.stderr.write("RX: %d\r" % count)			

			# clear file empty flag
			if fileempty:
				fileempty = 0
		if fileempty == 0 :
			# write payload
			for d in line.split(' '):
				# do a match because their might be a \r floating around
				m = re.match('.*(\w\w).*', d)
				if m is not None:
					outfile.write(pack('<B', int(m.group(1),16)))
					outfile.flush()
except KeyboardInterrupt:
#		cn.close()
		sys.exit(2)


