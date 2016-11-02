# Streaming
import socket
import numpy
import time
import cv2
import sys
import struct
import thread

# Stitching
from panorama import Stitcher
import argparse
import imutils
import cv2

UDP_IP = "127.0.0.1"
UDP_PORT_0 = 10000
UDP_PORT_1 = 10001

BUFF_LENGTH = 65540
PACK_SIZE = 4096

global_img_0 = None
global_img_1 = None

def socket_fetch(udp_port):
    global global_img_0
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind ((UDP_IP, udp_port))
    buff = ""
    data = ""
    while True:
        data = bytearray(BUFF_LENGTH) 
        data = sock.recv(BUFF_LENGTH)
        total_pack = ord(data[0])
        print total_pack
        buff = ""
        for i in xrange(total_pack):
            data = sock.recv(PACK_SIZE)
            buff += data
        frame = numpy.fromstring(buff, dtype=numpy.uint8)
        global_img_0 = cv2.imdecode(frame, 1)

def socket_fetch_1(udp_port):
    global global_img_1
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind ((UDP_IP, udp_port))
    buff = ""
    data = ""
    while True:
        data = bytearray(BUFF_LENGTH) 
        data = sock.recv(BUFF_LENGTH)
        total_pack = ord(data[0])
        buff = ""
        for i in xrange(total_pack):
            data = sock.recv(PACK_SIZE)
            buff += data
        frame = numpy.fromstring(buff, dtype=numpy.uint8)
        global_img_1 = cv2.imdecode(frame, 1)

def socket_fetch_test(udp_port):
    global global_img_1
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind ((UDP_IP, udp_port))
    buff = ""
    data = ""
    while True:
        data = sock.recv(BUFF_LENGTH)
        print sys.getsizeof(data)
        continue
        if sys.getsizeof(data) != 41:
            total_pack = struct.unpack("!i", data)[0]
            print total_pack
            continue
        total_pack = struct.unpack("!i", data)[0]
        buff = ""
        for i in xrange(total_pack):
            data = sock.recv(PACK_SIZE)
            buff += data
        frame = numpy.fromstring(buff, dtype=numpy.uint8)
        global_img_1 = cv2.imdecode(frame, 1)
        cv2.imshow('frame 0', global_img_1)
        if cv2.waitKey(1) & 0xFF == ord ('q'):
            break
socket_fetch_test(10000)

# try:
#    thread.start_new_thread(socket_fetch,(UDP_PORT_0,))
#    thread.start_new_thread(socket_fetch_1,(UDP_PORT_1,))
# except:
#    print "Error: unable to start thread"

# stitch the images together to create a panorama
stitcher = Stitcher()

# while True:
    # print global_img_0
    # if global_img_0 is not None:
    #     cv2.imshow('frame 0', global_img_0)
    #     if cv2.waitKey(1) & 0xFF == ord ('q'):
    #         break
    # print global_img_1
    # if global_img_1 is not None:
    #     img_1 = cv2.imdecode(global_img_1, 1)
    #     cv2.imshow('frame 1', global_img_1)
    #     if cv2.waitKey(1) & 0xFF == ord ('q'):
    #         break
    # if global_img_0 is not None and global_img_1 is not None:
    #     result = stitcher.stitch([global_img_0, global_img_1], showMatches=False)
    #     cv2.imshow("Result", result)
    #     if cv2.waitKey(1) & 0xFF == ord ('q'):
    #         break

