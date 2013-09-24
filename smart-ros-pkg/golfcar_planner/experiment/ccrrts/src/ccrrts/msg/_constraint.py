"""autogenerated by genpy from ccrrts/constraint.msg. Do not edit."""
import sys
python3 = True if sys.hexversion > 0x03000000 else False
import genpy
import struct

import std_msgs.msg

class constraint(genpy.Message):
  _md5sum = "8c545416a5d26bd7c1eccbf50602ceb8"
  _type = "ccrrts/constraint"
  _has_header = False #flag to mark the presence of a Header object
  _full_text = """Header head
int32 obst_id
int32[] serial_no
float64[] a_1
float64[] a_2
float64[] b
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

"""
  __slots__ = ['head','obst_id','serial_no','a_1','a_2','b']
  _slot_types = ['std_msgs/Header','int32','int32[]','float64[]','float64[]','float64[]']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.

    The available fields are:
       head,obst_id,serial_no,a_1,a_2,b

    :param args: complete set of field values, in .msg order
    :param kwds: use keyword arguments corresponding to message field names
    to set specific fields.
    """
    if args or kwds:
      super(constraint, self).__init__(*args, **kwds)
      #message fields cannot be None, assign default values for those that are
      if self.head is None:
        self.head = std_msgs.msg.Header()
      if self.obst_id is None:
        self.obst_id = 0
      if self.serial_no is None:
        self.serial_no = []
      if self.a_1 is None:
        self.a_1 = []
      if self.a_2 is None:
        self.a_2 = []
      if self.b is None:
        self.b = []
    else:
      self.head = std_msgs.msg.Header()
      self.obst_id = 0
      self.serial_no = []
      self.a_1 = []
      self.a_2 = []
      self.b = []

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
      _x = self
      buff.write(_struct_3I.pack(_x.head.seq, _x.head.stamp.secs, _x.head.stamp.nsecs))
      _x = self.head.frame_id
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.pack('<I%ss'%length, length, _x))
      buff.write(_struct_i.pack(self.obst_id))
      length = len(self.serial_no)
      buff.write(_struct_I.pack(length))
      pattern = '<%si'%length
      buff.write(struct.pack(pattern, *self.serial_no))
      length = len(self.a_1)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(struct.pack(pattern, *self.a_1))
      length = len(self.a_2)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(struct.pack(pattern, *self.a_2))
      length = len(self.b)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(struct.pack(pattern, *self.b))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    :param str: byte array of serialized message, ``str``
    """
    try:
      if self.head is None:
        self.head = std_msgs.msg.Header()
      end = 0
      _x = self
      start = end
      end += 12
      (_x.head.seq, _x.head.stamp.secs, _x.head.stamp.nsecs,) = _struct_3I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.head.frame_id = str[start:end].decode('utf-8')
      else:
        self.head.frame_id = str[start:end]
      start = end
      end += 4
      (self.obst_id,) = _struct_i.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%si'%length
      start = end
      end += struct.calcsize(pattern)
      self.serial_no = struct.unpack(pattern, str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.a_1 = struct.unpack(pattern, str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.a_2 = struct.unpack(pattern, str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.b = struct.unpack(pattern, str[start:end])
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
      _x = self
      buff.write(_struct_3I.pack(_x.head.seq, _x.head.stamp.secs, _x.head.stamp.nsecs))
      _x = self.head.frame_id
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.pack('<I%ss'%length, length, _x))
      buff.write(_struct_i.pack(self.obst_id))
      length = len(self.serial_no)
      buff.write(_struct_I.pack(length))
      pattern = '<%si'%length
      buff.write(self.serial_no.tostring())
      length = len(self.a_1)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(self.a_1.tostring())
      length = len(self.a_2)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(self.a_2.tostring())
      length = len(self.b)
      buff.write(_struct_I.pack(length))
      pattern = '<%sd'%length
      buff.write(self.b.tostring())
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    :param str: byte array of serialized message, ``str``
    :param numpy: numpy python module
    """
    try:
      if self.head is None:
        self.head = std_msgs.msg.Header()
      end = 0
      _x = self
      start = end
      end += 12
      (_x.head.seq, _x.head.stamp.secs, _x.head.stamp.nsecs,) = _struct_3I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.head.frame_id = str[start:end].decode('utf-8')
      else:
        self.head.frame_id = str[start:end]
      start = end
      end += 4
      (self.obst_id,) = _struct_i.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%si'%length
      start = end
      end += struct.calcsize(pattern)
      self.serial_no = numpy.frombuffer(str[start:end], dtype=numpy.int32, count=length)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.a_1 = numpy.frombuffer(str[start:end], dtype=numpy.float64, count=length)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.a_2 = numpy.frombuffer(str[start:end], dtype=numpy.float64, count=length)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      pattern = '<%sd'%length
      start = end
      end += struct.calcsize(pattern)
      self.b = numpy.frombuffer(str[start:end], dtype=numpy.float64, count=length)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill

_struct_I = genpy.struct_I
_struct_i = struct.Struct("<i")
_struct_3I = struct.Struct("<3I")