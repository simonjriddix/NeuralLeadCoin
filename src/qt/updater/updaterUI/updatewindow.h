// Copyright 2024 SimonJRiddix & NeuralLead
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SJR_UpdaterWindow_H
#define SJR_UpdaterWindow_H

#include <qt/updater/cautoupdatergithub.h>

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include <iostream>

class UpdateListenerNoUI : public CAutoUpdaterGithub::UpdateStatusListener
{
public:
    QString lastError = "";

    void onUpdateAvailable(const CAutoUpdaterGithub::ChangeLog& changelog) override;
    void onUpdateDownloadProgress(float percentageDownloaded) override;
    void onUpdateDownloadFinished() override;
    void onUpdateError(const QString& errorMessage) override;
};

class UpdaterWindow : public QMainWindow, public CAutoUpdaterGithub::UpdateStatusListener
{
    Q_OBJECT  // Required to enable signals and slots in Qt

public:
    explicit UpdaterWindow(QWidget *parent, std::string repoName, std::string currentVersion, int delayBeforeCheckUpdates = 2000, bool ShowUIIfDownloadAvaible = false);

    void updateProgressBar(float percentage);

    // Listner functions
    void onUpdateAvailable(const CAutoUpdaterGithub::ChangeLog& changelog) override;
    void onUpdateDownloadProgress(float percentageDownloaded) override;
    void onUpdateDownloadFinished() override;
    void onUpdateError(const QString& errorMessage) override;

    void setCss(QString style);

private Q_SLOT:
    void onbtnUpdateClicked();
    void startUpdateCheck();

private:
    CAutoUpdaterGithub* updater = nullptr;
    QPushButton* btnUpdate = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* lblVersion = nullptr;
    QLabel* lblChangeLog = nullptr;
    QLabel* lblError = nullptr;
    
    QString latestURL = "";
    
    void InitUI();
};

#endif // SJR_UpdaterWindow_H
