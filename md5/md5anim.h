#pragma once

struct md5_anim_hierarchy {
  const char * name;
  int parent_index;
  int flags;
  int start_index;
};

struct md5_anim_bounds {
  vec3 min;
  vec3 max;
};

struct md5_anim_base_frame {
  vec3 pos;
  vec4 orient;
};

struct md5_anim {
  int num_frames;
  int num_joints;
  int frame_rate;
  int num_animated_components;
  md5_anim_hierarchy * hierarchy;
  md5_anim_bounds * bounds;
  md5_anim_base_frame * base_frame;
  float ** frame;
};
