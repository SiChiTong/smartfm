## *********************************************************
## 
## File autogenerated for the sick_ldmrs package 
## by the dynamic_reconfigure package.
## Please do not edit.
## 
## ********************************************************/

##**********************************************************
## Software License Agreement (BSD License)
##
##  Copyright (c) 2008, Willow Garage, Inc.
##  All rights reserved.
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions
##  are met:
##
##   * Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   * Redistributions in binary form must reproduce the above
##     copyright notice, this list of conditions and the following
##     disclaimer in the documentation and/or other materials provided
##     with the distribution.
##   * Neither the name of the Willow Garage nor the names of its
##     contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
##  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
##  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
##  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
##  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
##  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
##  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
##  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
##  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
##  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
##  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
##  POSSIBILITY OF SUCH DAMAGE.
##**********************************************************/

config_description = [{'srcline': 15, 'description': 'The angle of the first range measurement.', 'max': 50, 'cconsttype': 'const int', 'ctype': 'int', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'start_angle', 'edit_method': '', 'default': 50, 'level': 1, 'min': -59, 'type': 'int'}, {'srcline': 16, 'description': 'The angle of the last range measurement.', 'max': 49, 'cconsttype': 'const int', 'ctype': 'int', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'end_angle', 'edit_method': '', 'default': -50, 'level': 1, 'min': -60, 'type': 'int'}, {'srcline': 17, 'description': 'Scan frequency, 0 = 12.5Hz, 1 = 25 Hz, 2 = 50 Hz', 'max': 2, 'cconsttype': 'const int', 'ctype': 'int', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'scan_frequency', 'edit_method': '', 'default': 2, 'level': 1, 'min': 0, 'type': 'int'}, {'srcline': 18, 'description': 'Sychronization offset angle in degrees', 'max': 179, 'cconsttype': 'const int', 'ctype': 'int', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'sync_angle_offset', 'edit_method': '', 'default': 0, 'level': 1, 'min': -180, 'type': 'int'}, {'srcline': 19, 'description': 'Constant or focussed angular resolution type (focussed valid for 12.5Hz scan freq only)', 'max': True, 'cconsttype': 'const bool', 'ctype': 'bool', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'constant_angular_res', 'edit_method': '', 'default': True, 'level': 1, 'min': False, 'type': 'bool'}, {'srcline': 20, 'description': 'Frame id of the sensor. Scan messages append scan number (0-3) to this', 'max': '', 'cconsttype': 'const char * const', 'ctype': 'std::string', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'frame_id_prefix', 'edit_method': '', 'default': '/ldmrs', 'level': 3, 'min': '', 'type': 'str'}, {'srcline': 21, 'description': 'Scan messages will use first echo if true, last echo otherwise', 'max': True, 'cconsttype': 'const bool', 'ctype': 'bool', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'use_first_echo', 'edit_method': '', 'default': False, 'level': 3, 'min': False, 'type': 'bool'}, {'srcline': 22, 'description': 'high values will smooth time more, low values will track noisy time more', 'max': 1.0, 'cconsttype': 'const double', 'ctype': 'double', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'time_smoothing_factor', 'edit_method': '', 'default': 0.96999999999999997, 'level': 3, 'min': 0.0, 'type': 'double'}, {'srcline': 23, 'description': 'allowed error (miliseconds) between smooth time and noisy time before step correction is applied, -1 disables time correction', 'max': 500, 'cconsttype': 'const int', 'ctype': 'int', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'time_error_threshold', 'edit_method': '', 'default': 10, 'level': 3, 'min': -1, 'type': 'int'}, {'srcline': 24, 'description': 'Issues a restart with the new parameter value. Note: none of the changes will take effect until this is set', 'max': True, 'cconsttype': 'const bool', 'ctype': 'bool', 'srcfile': '../cfg/ldmrs_config.cfg', 'name': 'apply_changes', 'edit_method': '', 'default': False, 'level': 3, 'min': False, 'type': 'bool'}]

min = {}
max = {}
defaults = {}
level = {}
type = {}
all_level = 0

for param in config_description:
    min[param['name']] = param['min']
    max[param['name']] = param['max']
    defaults[param['name']] = param['default']
    level[param['name']] = param['level']
    type[param['name']] = param['type']
    all_level = all_level | param['level']
