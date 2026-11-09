#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATOGGLEACCESSIBLE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATOGGLEACCESSIBLE_H_

class QWidget;

// 在控件构造时注册，单独使用控件也能获得无障碍接口。
void elaEnsureToggleAccessibility();
void elaNotifyToggleStateChanged(QWidget* widget);
void elaNotifyToggleNameChanged(QWidget* widget);

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATOGGLEACCESSIBLE_H_
