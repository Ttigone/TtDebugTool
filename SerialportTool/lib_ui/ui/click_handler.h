/*****************************************************************//**
 * \file   click_handler.h
 * \brief  点击句柄
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_CLICK_HANDLER_H
#define UI_CLICK_HANDLER_H

#include "text/text_entity.h"

class ClickHandler;
using ClickHandlerPtr = std::shared_ptr<ClickHandler>;

struct ClickContext {
  Qt::MouseButton button = Qt::LeftButton;
  QVariant value;
};

class ClickHandlerHost {
 protected:
   virtual void clickHandlerActiveChanged(const ClickHandlerPtr& action,
     bool active) {

  }

   virtual void clickHandlerPressedChanged(const ClickHandlerPtr& action,
     bool pressed) {

   }
   virtual ~ClickHandlerHost() {

   }
   friend class ClickHandler;
};

class ClickHandler {
 public:
  virtual ~ClickHandler();

  virtual void onClick(ClickContext context) const = 0;

  virtual QString url() const { return QString(); }

  virtual QString toolTip() const { return QString(); }

  virtual QString dragText() const { return QString(); }

  virtual QString copyToClipboardText() const { return QString(); }

  virtual QString copyToClipboardContextItemText() const { return QString(); }

  struct TextEntity {
    EntityType type = EntityType();
    QString data;
  };
  virtual TextEntity getTextEntity() const;

  void setProperty(int id, QVariant value);
  [[nodiscard]] const QVariant &property(int id) const;

  static bool setActive(const ClickHandlerPtr &p,
                        ClickHandlerHost *host = nullptr);

  static bool clearActive(ClickHandlerHost *host = nullptr);

  static void pressed();

  static ClickHandlerPtr unPressed();

  static ClickHandlerPtr getActive();
  static ClickHandlerPtr getPressed();

  static bool showAsActive(const ClickHandlerPtr &p);
  static bool showAsPressed(const ClickHandlerPtr &p);
  static void hostDestoryed(ClickHandlerHost &host);

 private:
  static ClickHandlerHost *active_host_;
  static ClickHandlerHost *pressed_host_;

  std::map<int, QVariant> properties_;
};


#endif  // UI_CLICK_HANDLER_H
