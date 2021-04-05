#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QAbstractNativeEventFilter>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    void CheckBatteryStatus();

private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
