#include "T_PageLifecycle.h"

#include "ElaDialog.h"
#include "ElaPlainTextEdit.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include <QHBoxLayout>
#include <QTextDocument>
#include <QVBoxLayout>

T_PageLifecycle::T_PageLifecycle(QWidget* parent) : T_BasePage(parent)
{
    setWindowTitle("懒加载编辑器");
    createCustomWidget("首次进入时才创建此页；输入内容后切换页面，可体验离开确认。返回时编辑内容仍会保留。");
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto* status = new ElaText("页面已创建，尚无未保存修改", 15, central);
    _editor = new ElaPlainTextEdit(central);
    _editor->setPlaceholderText("在这里输入内容，然后点击其他导航页或后退按钮。");
    _editor->setMinimumHeight(240);
    auto* saved = new ElaPushButton("标记为已保存", central);
    connect(saved, &ElaPushButton::clicked, this, [this]() {
        _editor->document()->setModified(false);
    });
    connect(_editor->document(), &QTextDocument::modificationChanged, status, [status](bool modified) {
        status->setText(modified ? "有未保存修改，离开前会确认" : "当前内容已标记为保存");
    });
    layout->addWidget(status);
    layout->addWidget(_editor);
    layout->addWidget(saved, 0, Qt::AlignLeft);
    layout->addWidget(new ElaText("此页面只演示状态管理，不会写入磁盘。", 13, central));
    layout->addStretch();
    addCentralWidget(central, true, true, 0);
}

bool T_PageLifecycle::confirmLeave(QWidget* dialogParent)
{
    if (!_editor->document()->isModified())
    {
        return true;
    }
    ElaDialog dialog(dialogParent);
    dialog.setWindowTitle("确认离开");
    dialog.resize(440, 190);
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 50, 20, 20);
    layout->addWidget(new ElaText("当前内容尚未保存。仍要切换页面吗？", 16, &dialog));
    layout->addWidget(new ElaText("返回此页面后，输入内容仍然保留。", 13, &dialog));
    auto* buttons = new QHBoxLayout;
    auto* stay = new ElaPushButton("继续编辑", &dialog);
    auto* leave = new ElaPushButton("仍然离开", &dialog);
    buttons->addStretch();
    buttons->addWidget(stay);
    buttons->addWidget(leave);
    layout->addLayout(buttons);
    connect(stay, &ElaPushButton::clicked, &dialog, &QDialog::reject);
    connect(leave, &ElaPushButton::clicked, &dialog, &QDialog::accept);
    stay->setFocus();
    return dialog.exec() == QDialog::Accepted;
}
