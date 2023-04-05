# ######################################################################
#  @file      multiple_bot_asc.py
# 
#     This file simulates 500 BotASCs connecting up to the MainASC so we 
#     can measure and accurately gauge what that would require in terms 
#     of memory utilization on the MainASC's STM32 platform.
#  
#  @brief   
#  
#  @note     
# 
#  @warning  
# 
#  @author  Nuertey Odzeyem
#  
#  @date    April 1st, 2023
# 
#  @copyright Copyright (c) 2023 Alert Innovation. All Rights Reserved.
# ######################################################################

#!/usr/bin/env python

import socket
#import threading
from threading import *

MAIN_ASC_IP_ADDRESS = '192.0.2.1'
MAIN_ASC_PORT_NUMBER = 5000

MESSAGE_SIZE_IN_BYTES = 20

ACK_STRING = 'Alive!'

# Note the exclusion of the index at the end of the range:
#botasc_indices = list(range(1, 501))
botasc_indices = list(range(1, 3))

def connect_to_mainasc(botasc_index):
    # TBD Nuertey Odzeyem; placeholder for debug.
    #
    #socket_descriptor = botasc_index
    socket_descriptor = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_descriptor.connect((MAIN_ASC_IP_ADDRESS, MAIN_ASC_PORT_NUMBER))

    # Usually, in Python, if one returns a dictionary, it is faster. In 
    # our case though, such nuances do not apply, but still the return 
    # nuances of Python 'routines/functions' bears noting:
    return socket_descriptor

def do_communication_with_mainasc(socket_descriptor):
    while True:
        mainasc_message = socket_descriptor.recv(MESSAGE_SIZE_IN_BYTES)
        print("Message received from MainASC:=> ")
        print(mainasc_message.decode("utf-8"))
        print()
        
        socket_descriptor.send(ACK_STRING)

print("Here! In the main multiple_bot_asc program... ")
print()

# A more pythonic way of achieving loop-like behavior--list comprehensions.
# And it is more efficient too! as it engenders and is interpreted into 
# faster processing code. Thus, employ a list comprehension to simulate
# the 500 BotASCs connecting up to the MainASC on the 500 sockets:
multiple_botasc_socket_descriptors = [connect_to_mainasc(botasc_index) for botasc_index in botasc_indices] 

print("Here are the socket descriptor numbers: ")
print()

print(multiple_botasc_socket_descriptors)
print()

# Now actually achieve the communication back and forth with the MainASC...
# All 500 sockets each with its own thread of execution; to put it as
# Rabelais would, "Each in its own eachery! :)"
for socket_descriptor in multiple_botasc_socket_descriptors:
    single_botasc_thread_name = 'BotASCThread_' + str(socket_descriptor)
    single_botasc_thread = threading.Thread(target=do_communication_with_mainasc, name=single_botasc_thread_name, args=(socket_descriptor,))
    
    # Being a daemon here implies thread will also be killed if the main thread context ends.
    single_botasc_thread.daemon = True 
    single_botasc_thread.start()

print("Main context, here after the threads; nothing to do. Awaiting the\
       multiple BotASC threads communicating continuously with the MainASC.")
print()
