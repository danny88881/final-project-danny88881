//
// Created by Danny on 11/25/2020.
//

#include "attack.h"
#include "world.h"
#include "cinder/app/App.h"

namespace final_project {

Attack::Attack() : Actor(vec2(0,0), vec2(0,0), Rect(0,0,0,0),
                          Rect(-32,-32,32,32), -1, -1, 0,
                          {false, false, false, false}, ActorType::kNeutral),
      damage_(1), sprite_sheet_path_("sprites/weapon/32slash.png"),
      max_frames_(1), frame_life_(0.5f), frame_index_(0), rotation_(0),
      hit_actors_() {
}

Attack::Attack(const Attack &attack, double rotation, vec2 position) {
  frame_index_ = 0;
  position_ = position;
  velocity_ = attack.velocity_;
  collision_ = attack.collision_;
  hit_box_ = attack.hit_box_;
  max_health_ = attack.max_health_;
  health_ = attack.health_;
  speed_ = attack.speed_;
  collision_layers_ = attack.collision_layers_;
  type_ = attack.type_;
  damage_ = attack.damage_;
  sprite_sheet_path_ = attack.sprite_sheet_path_;
  max_frames_ = attack.max_frames_;
  frame_life_ = attack.frame_life_;
  rotation_ = rotation;
  hit_actors_ = {};
}

void Attack::Setup(World &world) {
  int index = world.LoadTexture(sprite_sheet_path_);
  material_ = ci::gl::GlslProg::create(
      ci::gl::GlslProg::Format()
          .vertex(CI_GLSL(
                      150,
                      uniform mat4 ciModelViewProjection;
                          in vec4 ciPosition;
                          in vec2 ciTexCoord0;
                          out vec2 TexCoord0;
                          void main(void) {
                            gl_Position = ciModelViewProjection * ciPosition;
                            TexCoord0 = ciTexCoord0;
                          }))
          .fragment(CI_GLSL(
                        150,
                        uniform sampler2D uTex0;
                            uniform sampler2D uTex1;
                            uniform vec4 uColor;
                            uniform int frame;
                            uniform int max_frames;
                            in vec2 TexCoord0;
                            out vec4 oColor;
                            void main(void) {

                              float u = TexCoord0.x * (1.0 / max_frames)
                                  + (frame) * 1.0 / max_frames;

                              vec4 color = texture2D(uTex0,
                                                     vec2(u, TexCoord0.y))
                                  * uColor;
                              oColor = color;
                            }))
  );
  rect_ = ci::gl::Batch::create(ci::geom::Plane(), material_);
  ci::ColorAf color( 1,1,1,1 );
  material_->uniform("uTex0", index);
  material_->uniform("frame", 0);
  material_->uniform("max_frames", max_frames_);
  material_->uniform( "uColor", color );
}

void Attack::Update(float time_scale, World &world,
                    const InputController &controller) {
  ++frame_index_;
  if (frame_index_ >= frame_life_ * kFrameSkip) {
    world.RemoveActor(this);
    return;
  }
  vec2 direction = vec2(cos(rotation_), sin(rotation_));
  velocity_ = direction * vec2(speed_);

  position_ += velocity_ * time_scale;

  for (Actor *actor : world.GetActors()) {
    auto iter = std::find(hit_actors_.begin(), hit_actors_.end(), actor);
    if (iter != hit_actors_.end()) {
      continue;
    }
    if (actor != this && IsCollidingWithHitBox(*actor)) {
      actor->Damage(damage_);
      hit_actors_.push_back(actor);
    }
  }
}

void Attack::Draw() const {
  ci::gl::ScopedModelMatrix scpMtx;
  ci::gl::translate(2*(position_.x - ci::app::getWindowSize().x / 2)
                        /ci::app::getWindowSize().x,
                    0,
                    2*(position_.y - ci::app::getWindowSize().y / 2)
                        /ci::app::getWindowSize().y);

  int cur_frame = (int)std::floor((double)frame_index_ / kFrameSkip);
  material_->uniform("frame", cur_frame % (max_frames_));

  ci::gl::scale(0.125f,1,-0.125f);
  ci::gl::rotate((float)rotation_, glm::vec3(0,1,0));
  rect_->draw();
  ci::gl::setMatricesWindow(cinder::app::getWindowSize());
  ci::gl::color(1,0,0,1);
  ci::gl::drawLine(glm::vec2(hit_box_.x1_ + position_.x,hit_box_.y1_ + position_.y),
                   glm::vec2(hit_box_.x2_ + position_.x,hit_box_.y2_ + position_.y));
}

}