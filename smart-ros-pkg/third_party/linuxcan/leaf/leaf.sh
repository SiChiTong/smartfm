#!/bin/sh
# Kvaser CAN driver                     
# leaf.sh - start/stop leaf and create/delete device files  
# this script can be used if hotplugging doesn't work
#
#                 Copyright 2012 by Kvaser AB, M�lndal, Sweden
#                         http://www.kvaser.com
# 
#  This software is dual licensed under the following two licenses:
#  BSD-new and GPLv2. You may use either one. See the included
#  COPYING file for details.
# 
#  License: BSD-new
#  ===============================================================================
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of the <organization> nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
#  License: GPLv2
#  ===============================================================================
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#  ---------------------------------------------------------------------------
#

LOG=`which logger`

#     
# test kernel version
#     
kernel_major=`uname -r |cut -d \. -f 1` 
kernel_minor=`uname -r |cut -d \. -f 2` 

if [ $kernel_major = 2 ] && [ $kernel_minor = 4 ]; then
  kv_module_install=insmod
else
  kv_module_install=modprobe
fi

#
# Install
#

# Add or remove leaf module
case "$1" in
   start)
      /bin/sleep 3 # Sleep a second or two to be sure that module init is executed
      /sbin/rmmod leaf
      /sbin/$kv_module_install leaf || exit 1
      $LOG -t $0 "Module leaf added"
      nrchan=`cat /proc/leaf | grep 'total channels' | cut -d \  -f 3`
      major=`cat /proc/devices | grep 'leaf' | cut -d \  -f 1`
      rm -f /dev/leaf*
      for minor in $(seq 0 `expr $nrchan - 1`) ; do
         $LOG -t $0 "Created /dev/leaf$minor"
         mknod /dev/leaf$minor c $major $minor
      done
      ;;
   stop)
      /sbin/rmmod leaf || exit 1
      rm -f /dev/leaf*
      $LOG -t $0 "Module leaf removed"
      ;;
   restart)
      rm -f /dev/leaf*
      /sbin/$kv_module_install leaf || exit 1
      $LOG -t $0 "Module leaf added"
      nrchan=`cat /proc/leaf | grep 'total channels' | cut -d \  -f 3`
      major=`cat /proc/devices | grep 'leaf' | cut -d \  -f 1`
      rm -f /dev/leaf*
      for minor in $(seq 0 `expr $nrchan - 1`) ; do
         $LOG -t $0 "Created /dev/leaf$minor"
         mknod /dev/leaf$minor c $major $minor
      done
      ;;
   *)
      printf "Usage: %s {start|stop|restart}\n" $0
esac

exit 0 
