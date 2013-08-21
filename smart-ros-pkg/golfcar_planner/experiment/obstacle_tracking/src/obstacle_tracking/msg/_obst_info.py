"""autogenerated by genpy from obstacle_tracking/obst_info.msg. Do not edit."""
import sys
python3 = True if sys.hexversion > 0x03000000 else False
import genpy
import struct

import geometry_msgs.msg
import std_msgs.msg

class obst_info(genpy.Message):
  _md5sum = "0fc059335f3f0c3a0de1cb4189b756f1"
  _type = "obstacle_tracking/obst_info"
  _has_header = False #flag to mark the presence of a Header object
  _full_text = """geometry_msgs/PolygonStamped[] veh_polys
geometry_msgs/Point32[] ped_pts
================================================================================
MSG: geometry_msgs/PolygonStamped
# This represents a Polygon with reference coordinate frame and timestamp
Header header
Polygon polygon

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.secs: seconds (stamp_secs) since epoch
# * stamp.nsecs: nanoseconds since stamp_secs
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
# 0: no frame
# 1: global frame
string frame_id

================================================================================
MSG: geometry_msgs/Polygon
#A specification of a polygon where the first and last points are assumed to be connected
Point32[] points

================================================================================
MSG: geometry_msgs/Point32
# This contains the position of a point in free space(with 32 bits of precision).
# It is recommeded to use Point wherever possible instead of Point32.  
# 
# This recommendation is to promote interoperability.  
#
# This message is designed to take up less space when sending
# lots of points at once, as in the case of a PointCloud.  

float32 x
float32 y
float32 z
"""
  __slots__ = ['veh_polys','ped_pts']
  _slot_types = ['geometry_msgs/PolygonStamped[]','geometry_msgs/Point32[]']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.

    The available fields are:
       veh_polys,ped_pts

    :param args: complete set of field values, in .msg order
    :param kwds: use keyword arguments corresponding to message field names
    to set specific fields.
    """
    if args or kwds:
      super(obst_info, self).__init__(*args, **kwds)
      #message fields cannot be None, assign default values for those that are
      if self.veh_polys is None:
        self.veh_polys = []
      if self.ped_pts is None:
        self.ped_pts = []
    else:
      self.veh_polys = []
      self.ped_pts = []

  def _get_types(self):
    """
    internal API method
    """
    return self._slot_types

  def serialize(self, buff):
    """
    serialize message into buffer
    :param buff: buffer, ``StringIO``
    """
    try:
      length = len(self.veh_polys)
      buff.write(_struct_I.pack(length))
      for val1 in self.veh_polys:
        _v1 = val1.header
        buff.write(_struct_I.pack(_v1.seq))
        _v2 = _v1.stamp
        _x = _v2
        buff.write(_struct_2I.pack(_x.secs, _x.nsecs))
        _x = _v1.frame_id
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _v3 = val1.polygon
        length = len(_v3.points)
        buff.write(_struct_I.pack(length))
        for val3 in _v3.points:
          _x = val3
          buff.write(_struct_3f.pack(_x.x, _x.y, _x.z))
      length = len(self.ped_pts)
      buff.write(_struct_I.pack(length))
      for val1 in self.ped_pts:
        _x = val1
        buff.write(_struct_3f.pack(_x.x, _x.y, _x.z))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    :param str: byte array of serialized message, ``str``
    """
    try:
      if self.veh_polys is None:
        self.veh_polys = None
      if self.ped_pts is None:
        self.ped_pts = None
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.veh_polys = []
      for i in range(0, length):
        val1 = geometry_msgs.msg.PolygonStamped()
        _v4 = val1.header
        start = end
        end += 4
        (_v4.seq,) = _struct_I.unpack(str[start:end])
        _v5 = _v4.stamp
        _x = _v5
        start = end
        end += 8
        (_x.secs, _x.nsecs,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          _v4.frame_id = str[start:end].decode('utf-8')
        else:
          _v4.frame_id = str[start:end]
        _v6 = val1.polygon
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        _v6.points = []
        for i in range(0, length):
          val3 = geometry_msgs.msg.Point32()
          _x = val3
          start = end
          end += 12
          (_x.x, _x.y, _x.z,) = _struct_3f.unpack(str[start:end])
          _v6.points.append(val3)
        self.veh_polys.append(val1)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.ped_pts = []
      for i in range(0, length):
        val1 = geometry_msgs.msg.Point32()
        _x = val1
        start = end
        end += 12
        (_x.x, _x.y, _x.z,) = _struct_3f.unpack(str[start:end])
        self.ped_pts.append(val1)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill


  def serialize_numpy(self, buff, numpy):
    """
    serialize message with numpy array types into buffer
    :param buff: buffer, ``StringIO``
    :param numpy: numpy python module
    """
    try:
      length = len(self.veh_polys)
      buff.write(_struct_I.pack(length))
      for val1 in self.veh_polys:
        _v7 = val1.header
        buff.write(_struct_I.pack(_v7.seq))
        _v8 = _v7.stamp
        _x = _v8
        buff.write(_struct_2I.pack(_x.secs, _x.nsecs))
        _x = _v7.frame_id
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _v9 = val1.polygon
        length = len(_v9.points)
        buff.write(_struct_I.pack(length))
        for val3 in _v9.points:
          _x = val3
          buff.write(_struct_3f.pack(_x.x, _x.y, _x.z))
      length = len(self.ped_pts)
      buff.write(_struct_I.pack(length))
      for val1 in self.ped_pts:
        _x = val1
        buff.write(_struct_3f.pack(_x.x, _x.y, _x.z))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    :param str: byte array of serialized message, ``str``
    :param numpy: numpy python module
    """
    try:
      if self.veh_polys is None:
        self.veh_polys = None
      if self.ped_pts is None:
        self.ped_pts = None
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.veh_polys = []
      for i in range(0, length):
        val1 = geometry_msgs.msg.PolygonStamped()
        _v10 = val1.header
        start = end
        end += 4
        (_v10.seq,) = _struct_I.unpack(str[start:end])
        _v11 = _v10.stamp
        _x = _v11
        start = end
        end += 8
        (_x.secs, _x.nsecs,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          _v10.frame_id = str[start:end].decode('utf-8')
        else:
          _v10.frame_id = str[start:end]
        _v12 = val1.polygon
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        _v12.points = []
        for i in range(0, length):
          val3 = geometry_msgs.msg.Point32()
          _x = val3
          start = end
          end += 12
          (_x.x, _x.y, _x.z,) = _struct_3f.unpack(str[start:end])
          _v12.points.append(val3)
        self.veh_polys.append(val1)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.ped_pts = []
      for i in range(0, length):
        val1 = geometry_msgs.msg.Point32()
        _x = val1
        start = end
        end += 12
        (_x.x, _x.y, _x.z,) = _struct_3f.unpack(str[start:end])
        self.ped_pts.append(val1)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill

_struct_I = genpy.struct_I
_struct_2I = struct.Struct("<2I")
_struct_3f = struct.Struct("<3f")
