#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QAbstractNativeEventFilter>

// 從標準程式庫中引入 string
#include <string>

// 使用 std 中的 string 名稱
using std::string;

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
    void getWindowTitle();

    // QString 與 string 的轉換函數
    QString s2q(const string &);
    string q2s(const QString &);

    void QStringformat();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
