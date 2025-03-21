#include "ui/click_handler.h"

namespace
{
ClickHandlerPtr &ClickHandlerActive() {
  static auto result = ClickHandlerPtr();
  return result;
}

ClickHandlerPtr &ClickHandlerPressed() {
  static auto result = ClickHandlerPtr();
  return result;
}
}


ClickHandlerHost *ClickHandler::active_host_ = nullptr;
ClickHandlerHost *ClickHandler::pressed_host_ = nullptr;

ClickHandler::~ClickHandler()
{}

ClickHandler::TextEntity ClickHandler::getTextEntity() const
{
  return ClickHandler::TextEntity();
}

bool ClickHandler::setActive(const ClickHandlerPtr& p, ClickHandlerHost* host)
{
  return false;
}

bool ClickHandler::clearActive(ClickHandlerHost* host)
{ return false; }

void ClickHandler::pressed() {
  auto &active = ClickHandlerActive();
  auto &pressed = ClickHandlerPressed();
  unPressed();
  if (!active) {
    return;
  }
  pressed = active;
  if (pressed_host_ = active_host_) {
    pressed_host_->clickHandlerPressedChanged(pressed, true);
  }
}

ClickHandlerPtr ClickHandler::unPressed()
{
  auto &active = ClickHandlerActive();
  auto &pressed = ClickHandlerPressed();

  if (pressed) {
    const auto activated = (active == pressed);
    //const auto waspressed = base::take(pressed);
    const auto waspressed = pressed;
    if (pressed_host_) {
      pressed_host_->clickHandlerPressedChanged(waspressed, false);
      pressed_host_ = nullptr;
    }

    if (activated) {
      return active;
    } else if (active && active_host_) {
      // emit clickHandlerActiveChanged for current active
      // click handler, which we didn't emit while we has
      // a pressed click handler
      active_host_->clickHandlerActiveChanged(active, true);
    }
  }
  return ClickHandlerPtr();
}

ClickHandlerPtr ClickHandler::getActive()
{ return ClickHandlerActive(); }

ClickHandlerPtr ClickHandler::getPressed()
{
  return ClickHandlerPressed();
}

bool ClickHandler::showAsActive(const ClickHandlerPtr& p)
{
  auto &active = ClickHandlerActive();
  auto &pressed = ClickHandlerPressed();

  return p && (p == active) && (!pressed || (p == pressed));
}

bool ClickHandler::showAsPressed(const ClickHandlerPtr& p)
{
  auto &active = ClickHandlerActive();
  auto &pressed = ClickHandlerPressed();

  return p && (p == active) && (p == pressed);
}

void ClickHandler::hostDestoryed(ClickHandlerHost& host)
{
  auto &active = ClickHandlerActive();
  auto &pressed = ClickHandlerPressed();

  //if (active_host_ == host) {
  //  active = nullptr;
  //  active_host_= nullptr;
  //}
  //if (pressed_host_== host) {
  //  pressed = nullptr;
  //  pressed_host_= nullptr;
  //}
}
