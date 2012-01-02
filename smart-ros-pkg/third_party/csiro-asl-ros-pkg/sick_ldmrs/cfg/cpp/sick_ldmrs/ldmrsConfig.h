//#line 2 "/opt/ros/electric/stacks/driver_common/dynamic_reconfigure/templates/ConfigType.h"
// *********************************************************
// 
// File autogenerated for the sick_ldmrs package 
// by the dynamic_reconfigure package.
// Please do not edit.
// 
// ********************************************************/

/***********************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************/

// Author: Blaise Gassend


#ifndef __sick_ldmrs__LDMRSCONFIG_H__
#define __sick_ldmrs__LDMRSCONFIG_H__

#include <dynamic_reconfigure/config_tools.h>
#include <limits>
#include <ros/node_handle.h>
#include <dynamic_reconfigure/ConfigDescription.h>
#include <dynamic_reconfigure/ParamDescription.h>
#include <dynamic_reconfigure/config_init_mutex.h>

namespace sick_ldmrs
{
  class ldmrsConfigStatics;
  
  class ldmrsConfig
  {
  public:
    class AbstractParamDescription : public dynamic_reconfigure::ParamDescription
    {
    public:
      AbstractParamDescription(std::string n, std::string t, uint32_t l, 
          std::string d, std::string e)
      {
        name = n;
        type = t;
        level = l;
        description = d;
        edit_method = e;
      }
      
      virtual void clamp(ldmrsConfig &config, const ldmrsConfig &max, const ldmrsConfig &min) const = 0;
      virtual void calcLevel(uint32_t &level, const ldmrsConfig &config1, const ldmrsConfig &config2) const = 0;
      virtual void fromServer(const ros::NodeHandle &nh, ldmrsConfig &config) const = 0;
      virtual void toServer(const ros::NodeHandle &nh, const ldmrsConfig &config) const = 0;
      virtual bool fromMessage(const dynamic_reconfigure::Config &msg, ldmrsConfig &config) const = 0;
      virtual void toMessage(dynamic_reconfigure::Config &msg, const ldmrsConfig &config) const = 0;
    };

    typedef boost::shared_ptr<AbstractParamDescription> AbstractParamDescriptionPtr;
    typedef boost::shared_ptr<const AbstractParamDescription> AbstractParamDescriptionConstPtr;
    
    template <class T>
    class ParamDescription : public AbstractParamDescription
    {
    public:
      ParamDescription(std::string name, std::string type, uint32_t level, 
          std::string description, std::string edit_method, T ldmrsConfig::* f) :
        AbstractParamDescription(name, type, level, description, edit_method),
        field(f)
      {}

      T (ldmrsConfig::* field);

      virtual void clamp(ldmrsConfig &config, const ldmrsConfig &max, const ldmrsConfig &min) const
      {
        if (config.*field > max.*field)
          config.*field = max.*field;
        
        if (config.*field < min.*field)
          config.*field = min.*field;
      }

      virtual void calcLevel(uint32_t &comb_level, const ldmrsConfig &config1, const ldmrsConfig &config2) const
      {
        if (config1.*field != config2.*field)
          comb_level |= level;
      }

      virtual void fromServer(const ros::NodeHandle &nh, ldmrsConfig &config) const
      {
        nh.getParam(name, config.*field);
      }

      virtual void toServer(const ros::NodeHandle &nh, const ldmrsConfig &config) const
      {
        nh.setParam(name, config.*field);
      }

      virtual bool fromMessage(const dynamic_reconfigure::Config &msg, ldmrsConfig &config) const
      {
        return dynamic_reconfigure::ConfigTools::getParameter(msg, name, config.*field);
      }

      virtual void toMessage(dynamic_reconfigure::Config &msg, const ldmrsConfig &config) const
      {
        dynamic_reconfigure::ConfigTools::appendParameter(msg, name, config.*field);
      }
    };

//#line 15 "../cfg/ldmrs_config.cfg"
      int start_angle;
//#line 16 "../cfg/ldmrs_config.cfg"
      int end_angle;
//#line 17 "../cfg/ldmrs_config.cfg"
      int scan_frequency;
//#line 18 "../cfg/ldmrs_config.cfg"
      int sync_angle_offset;
//#line 19 "../cfg/ldmrs_config.cfg"
      bool constant_angular_res;
//#line 20 "../cfg/ldmrs_config.cfg"
      std::string frame_id_prefix;
//#line 21 "../cfg/ldmrs_config.cfg"
      bool use_first_echo;
//#line 22 "../cfg/ldmrs_config.cfg"
      double time_smoothing_factor;
//#line 23 "../cfg/ldmrs_config.cfg"
      int time_error_threshold;
//#line 24 "../cfg/ldmrs_config.cfg"
      bool apply_changes;
//#line 138 "/opt/ros/electric/stacks/driver_common/dynamic_reconfigure/templates/ConfigType.h"

    bool __fromMessage__(dynamic_reconfigure::Config &msg)
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      int count = 0;
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        if ((*i)->fromMessage(msg, *this))
          count++;
      if (count != dynamic_reconfigure::ConfigTools::size(msg))
      {
        ROS_ERROR("ldmrsConfig::__fromMessage__ called with an unexpected parameter.");
        ROS_ERROR("Booleans:");
        for (unsigned int i = 0; i < msg.bools.size(); i++)
          ROS_ERROR("  %s", msg.bools[i].name.c_str());
        ROS_ERROR("Integers:");
        for (unsigned int i = 0; i < msg.ints.size(); i++)
          ROS_ERROR("  %s", msg.ints[i].name.c_str());
        ROS_ERROR("Doubles:");
        for (unsigned int i = 0; i < msg.doubles.size(); i++)
          ROS_ERROR("  %s", msg.doubles[i].name.c_str());
        ROS_ERROR("Strings:");
        for (unsigned int i = 0; i < msg.strs.size(); i++)
          ROS_ERROR("  %s", msg.strs[i].name.c_str());
        // @todo Check that there are no duplicates. Make this error more
        // explicit.
        return false;
      }
      return true;
    }

    // This version of __toMessage__ is used during initialization of
    // statics when __getParamDescriptions__ can't be called yet.
    void __toMessage__(dynamic_reconfigure::Config &msg, const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__) const
    {
      dynamic_reconfigure::ConfigTools::clear(msg);
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        (*i)->toMessage(msg, *this);
    }
    
    void __toMessage__(dynamic_reconfigure::Config &msg) const
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      __toMessage__(msg, __param_descriptions__);
    }
    
    void __toServer__(const ros::NodeHandle &nh) const
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        (*i)->toServer(nh, *this);
    }

    void __fromServer__(const ros::NodeHandle &nh)
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        (*i)->fromServer(nh, *this);
    }

    void __clamp__()
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      const ldmrsConfig &__max__ = __getMax__();
      const ldmrsConfig &__min__ = __getMin__();
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        (*i)->clamp(*this, __max__, __min__);
    }

    uint32_t __level__(const ldmrsConfig &config) const
    {
      const std::vector<AbstractParamDescriptionConstPtr> &__param_descriptions__ = __getParamDescriptions__();
      uint32_t level = 0;
      for (std::vector<AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        (*i)->calcLevel(level, config, *this);
      return level;
    }
    
    static const dynamic_reconfigure::ConfigDescription &__getDescriptionMessage__();
    static const ldmrsConfig &__getDefault__();
    static const ldmrsConfig &__getMax__();
    static const ldmrsConfig &__getMin__();
    static const std::vector<AbstractParamDescriptionConstPtr> &__getParamDescriptions__();
    
  private:
    static const ldmrsConfigStatics *__get_statics__();
  };
  
  template <> // Max and min are ignored for strings.
  inline void ldmrsConfig::ParamDescription<std::string>::clamp(ldmrsConfig &config, const ldmrsConfig &max, const ldmrsConfig &min) const
  {
    return;
  }

  class ldmrsConfigStatics
  {
    friend class ldmrsConfig;
    
    ldmrsConfigStatics()
    {
//#line 15 "../cfg/ldmrs_config.cfg"
      __min__.start_angle = -59;
//#line 15 "../cfg/ldmrs_config.cfg"
      __max__.start_angle = 50;
//#line 15 "../cfg/ldmrs_config.cfg"
      __default__.start_angle = 50;
//#line 15 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<int>("start_angle", "int", 1, "The angle of the first range measurement.", "", &ldmrsConfig::start_angle)));
//#line 16 "../cfg/ldmrs_config.cfg"
      __min__.end_angle = -60;
//#line 16 "../cfg/ldmrs_config.cfg"
      __max__.end_angle = 49;
//#line 16 "../cfg/ldmrs_config.cfg"
      __default__.end_angle = -50;
//#line 16 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<int>("end_angle", "int", 1, "The angle of the last range measurement.", "", &ldmrsConfig::end_angle)));
//#line 17 "../cfg/ldmrs_config.cfg"
      __min__.scan_frequency = 0;
//#line 17 "../cfg/ldmrs_config.cfg"
      __max__.scan_frequency = 2;
//#line 17 "../cfg/ldmrs_config.cfg"
      __default__.scan_frequency = 2;
//#line 17 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<int>("scan_frequency", "int", 1, "Scan frequency, 0 = 12.5Hz, 1 = 25 Hz, 2 = 50 Hz", "", &ldmrsConfig::scan_frequency)));
//#line 18 "../cfg/ldmrs_config.cfg"
      __min__.sync_angle_offset = -180;
//#line 18 "../cfg/ldmrs_config.cfg"
      __max__.sync_angle_offset = 179;
//#line 18 "../cfg/ldmrs_config.cfg"
      __default__.sync_angle_offset = 0;
//#line 18 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<int>("sync_angle_offset", "int", 1, "Sychronization offset angle in degrees", "", &ldmrsConfig::sync_angle_offset)));
//#line 19 "../cfg/ldmrs_config.cfg"
      __min__.constant_angular_res = 0;
//#line 19 "../cfg/ldmrs_config.cfg"
      __max__.constant_angular_res = 1;
//#line 19 "../cfg/ldmrs_config.cfg"
      __default__.constant_angular_res = 1;
//#line 19 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<bool>("constant_angular_res", "bool", 1, "Constant or focussed angular resolution type (focussed valid for 12.5Hz scan freq only)", "", &ldmrsConfig::constant_angular_res)));
//#line 20 "../cfg/ldmrs_config.cfg"
      __min__.frame_id_prefix = "";
//#line 20 "../cfg/ldmrs_config.cfg"
      __max__.frame_id_prefix = "";
//#line 20 "../cfg/ldmrs_config.cfg"
      __default__.frame_id_prefix = "/ldmrs";
//#line 20 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<std::string>("frame_id_prefix", "str", 3, "Frame id of the sensor. Scan messages append scan number (0-3) to this", "", &ldmrsConfig::frame_id_prefix)));
//#line 21 "../cfg/ldmrs_config.cfg"
      __min__.use_first_echo = 0;
//#line 21 "../cfg/ldmrs_config.cfg"
      __max__.use_first_echo = 1;
//#line 21 "../cfg/ldmrs_config.cfg"
      __default__.use_first_echo = 0;
//#line 21 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<bool>("use_first_echo", "bool", 3, "Scan messages will use first echo if true, last echo otherwise", "", &ldmrsConfig::use_first_echo)));
//#line 22 "../cfg/ldmrs_config.cfg"
      __min__.time_smoothing_factor = 0.0;
//#line 22 "../cfg/ldmrs_config.cfg"
      __max__.time_smoothing_factor = 1.0;
//#line 22 "../cfg/ldmrs_config.cfg"
      __default__.time_smoothing_factor = 0.97;
//#line 22 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<double>("time_smoothing_factor", "double", 3, "high values will smooth time more, low values will track noisy time more", "", &ldmrsConfig::time_smoothing_factor)));
//#line 23 "../cfg/ldmrs_config.cfg"
      __min__.time_error_threshold = -1;
//#line 23 "../cfg/ldmrs_config.cfg"
      __max__.time_error_threshold = 500;
//#line 23 "../cfg/ldmrs_config.cfg"
      __default__.time_error_threshold = 10;
//#line 23 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<int>("time_error_threshold", "int", 3, "allowed error (miliseconds) between smooth time and noisy time before step correction is applied, -1 disables time correction", "", &ldmrsConfig::time_error_threshold)));
//#line 24 "../cfg/ldmrs_config.cfg"
      __min__.apply_changes = 0;
//#line 24 "../cfg/ldmrs_config.cfg"
      __max__.apply_changes = 1;
//#line 24 "../cfg/ldmrs_config.cfg"
      __default__.apply_changes = 0;
//#line 24 "../cfg/ldmrs_config.cfg"
      __param_descriptions__.push_back(ldmrsConfig::AbstractParamDescriptionConstPtr(new ldmrsConfig::ParamDescription<bool>("apply_changes", "bool", 3, "Issues a restart with the new parameter value. Note: none of the changes will take effect until this is set", "", &ldmrsConfig::apply_changes)));
//#line 239 "/opt/ros/electric/stacks/driver_common/dynamic_reconfigure/templates/ConfigType.h"
    
      for (std::vector<ldmrsConfig::AbstractParamDescriptionConstPtr>::const_iterator i = __param_descriptions__.begin(); i != __param_descriptions__.end(); i++)
        __description_message__.parameters.push_back(**i);
      __max__.__toMessage__(__description_message__.max, __param_descriptions__); 
      __min__.__toMessage__(__description_message__.min, __param_descriptions__); 
      __default__.__toMessage__(__description_message__.dflt, __param_descriptions__); 
    }
    std::vector<ldmrsConfig::AbstractParamDescriptionConstPtr> __param_descriptions__;
    ldmrsConfig __max__;
    ldmrsConfig __min__;
    ldmrsConfig __default__;
    dynamic_reconfigure::ConfigDescription __description_message__;
    static const ldmrsConfigStatics *get_instance()
    {
      // Split this off in a separate function because I know that
      // instance will get initialized the first time get_instance is
      // called, and I am guaranteeing that get_instance gets called at
      // most once.
      static ldmrsConfigStatics instance;
      return &instance;
    }
  };

  inline const dynamic_reconfigure::ConfigDescription &ldmrsConfig::__getDescriptionMessage__() 
  {
    return __get_statics__()->__description_message__;
  }

  inline const ldmrsConfig &ldmrsConfig::__getDefault__()
  {
    return __get_statics__()->__default__;
  }
  
  inline const ldmrsConfig &ldmrsConfig::__getMax__()
  {
    return __get_statics__()->__max__;
  }
  
  inline const ldmrsConfig &ldmrsConfig::__getMin__()
  {
    return __get_statics__()->__min__;
  }
  
  inline const std::vector<ldmrsConfig::AbstractParamDescriptionConstPtr> &ldmrsConfig::__getParamDescriptions__()
  {
    return __get_statics__()->__param_descriptions__;
  }

  inline const ldmrsConfigStatics *ldmrsConfig::__get_statics__()
  {
    const static ldmrsConfigStatics *statics;
  
    if (statics) // Common case
      return statics;

    boost::mutex::scoped_lock lock(dynamic_reconfigure::__init_mutex__);

    if (statics) // In case we lost a race.
      return statics;

    statics = ldmrsConfigStatics::get_instance();
    
    return statics;
  }


}

#endif // __LDMRSRECONFIGURATOR_H__