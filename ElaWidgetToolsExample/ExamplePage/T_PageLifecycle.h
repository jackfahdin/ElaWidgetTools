#ifndef ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_PAGELIFECYCLE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_PAGELIFECYCLE_H_

#include "T_BasePage.h"
class ElaPlainTextEdit;
class T_PageLifecycle : public T_BasePage
{
    Q_OBJECT
public:
    explicit T_PageLifecycle(QWidget* parent = nullptr);
    bool confirmLeave(QWidget* dialogParent);

private:
    ElaPlainTextEdit* _editor{nullptr};
};
#endif // ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_PAGELIFECYCLE_H_
