// Copyright 2024 SimonJRiddix & NeuralLead
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <qt/updater/updaterUI/updatewindow.h>
#include <QTimer>

#define DEBUG_UPDATE true

// No ui

void UpdateListenerNoUI::onUpdateAvailable(const CAutoUpdaterGithub::ChangeLog& changelog)
{
#if defined(DEBUG_UPDATE)
    std::cout << "Update avalaibles: " << changelog.size() << " founds." << std::endl;
#endif

    if(changelog.size() == 0)
    {
        // no updates founds...
    }

    // updates founds...
}

void UpdateListenerNoUI::onUpdateDownloadProgress(float percentageDownloaded)
{

}

void UpdateListenerNoUI::onUpdateDownloadFinished()
{
        
}

void UpdateListenerNoUI::onUpdateError(const QString& errorMessage)
{
    lastError = errorMessage;

#if defined(DEBUG_UPDATE)
    std::cout << "Update error: " << errorMessage.toStdString() << std::endl;
#endif
}

// with UI

UpdaterWindow::UpdaterWindow(QWidget *parent, std::string repoName, std::string currentVersion, int delayBeforeCheckUpdates, bool ShowUIIfDownloadAvaible) : QMainWindow(parent)
{
    QString qrepoName = repoName.c_str();
    QString qcurrentVersion = currentVersion.c_str();

    // Creazione e configurazione dell'updater
    updater = new CAutoUpdaterGithub(qrepoName, qcurrentVersion); // Set your github username and repo, set also the current version of your running software
    updater->setUpdateStatusListener(this);

    if(!ShowUIIfDownloadAvaible)
    {
        InitUI();
    }
    else
    {
        this->hide();
    }
    
    // Imposta un timer per ritardare il controllo degli aggiornamenti di 3 secondi
    QTimer::singleShot(delayBeforeCheckUpdates, this, &UpdaterWindow::startUpdateCheck);
}

void UpdaterWindow::InitUI()
{
    // Creiamo i widget
    lblVersion = new QLabel("Checking version...", this);
    lblChangeLog = new QLabel("", this);
    btnUpdate = new QPushButton("Update Now", this);
    btnUpdate->setEnabled(false);
    lblError = new QLabel("", this);
    progressBar = new QProgressBar(this);

    // Impostiamo la dimensione della QLabel del changelog
    lblChangeLog->setMinimumHeight(100);    // Altezza minima per rendere grande la QLabel
    lblChangeLog->setWordWrap(true);        // Abilita il word wrap per il testo

    // Configuriamo l'etichetta di errore
    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblError->setPalette(palette);

    // Configuriamo la progress bar
    progressBar->setRange(0, 100);          // Range da 0 a 100%
    progressBar->setValue(0);               // Imposta il valore iniziale a 0
    progressBar->setTextVisible(false);     // Nasconde il testo nella progress bar
    progressBar->setVisible(false);

    // Layout principale verticale
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(lblVersion);
    mainLayout->addWidget(lblChangeLog);
    mainLayout->addStretch();  // Aggiungiamo uno spazio flessibile per spingere il bottone e l'etichetta in basso

    // Layout orizzontale per la lblError e btnUpdate
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(lblError, 0, Qt::AlignLeft);    // Ancorata in basso a sinistra
    bottomLayout->addWidget(btnUpdate, 0, Qt::AlignRight);  // Ancorato in basso a destra

    mainLayout->addLayout(bottomLayout);
    mainLayout->addWidget(progressBar);  // Aggiungi la progress bar

    // Widget centrale con il layout
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Impostiamo il titolo della finestra
    setWindowTitle("NeuralLead Coin - update");

    // Impostiamo la dimensione della finestra
    resize(600, 500);

    // Connettiamo il bottone all'azione di aggiornamento
    connect(btnUpdate, &QPushButton::clicked, this, &UpdaterWindow::onbtnUpdateClicked);
    
    this->show();
}

void UpdaterWindow::onbtnUpdateClicked()
{
    if (btnUpdate)
        btnUpdate->setEnabled(false);
    if (progressBar)
        progressBar->setVisible(true);
    
    // uncomment to override download URL
    //listener->latestURL = "https://your-link.com";
    
    updater->downloadAndInstallUpdate(latestURL);
}

void UpdaterWindow::startUpdateCheck()
{
    updater->checkForUpdates();
}

void UpdaterWindow::updateProgressBar(float percentage)
{
    if(progressBar)
        progressBar->setValue(static_cast<int>(percentage));
}

// ---
// Listner functions
// ---

void UpdaterWindow::onUpdateAvailable(const CAutoUpdaterGithub::ChangeLog& changelog)
{
    if(lblError != nullptr)
        lblError->setText("");

#if defined(DEBUG_UPDATE)
        std::cout << "Update avalaibles: " << changelog.size() << " founds." << std::endl;
#endif

	if(changelog.size() == 0)
	{
        this->close();
        return;
	}

    if (btnUpdate == nullptr) // this is mean 'ShowUIIfDownloadAvaible' is set to true
    {
        // then init now ui
        InitUI();
    }
    
    for(auto clog : changelog)
	{
		latestURL = clog.versionUpdateUrl;
		lblChangeLog->setText("Download from: " + latestURL + "\n\n" + clog.versionChanges);
		lblVersion->setText("New version available " + clog.versionString);
		btnUpdate->setEnabled(true);
	}
}

void UpdaterWindow::onUpdateDownloadProgress(float percentageDownloaded)
{
#if defined(DEBUG_UPDATE)
    std::cout << "Download progress: " << percentageDownloaded << "%" << std::endl;
#endif
	updateProgressBar(percentageDownloaded);
}

void UpdaterWindow::onUpdateDownloadFinished()
{
#if defined(DEBUG_UPDATE)
    std::cout << "Download complete" << std::endl;
#endif
    if (btnUpdate)
	    btnUpdate->setEnabled(true);
}

void UpdaterWindow::onUpdateError(const QString& errorMessage)
{
    if (lblError)
        lblError->setText("Error " + errorMessage);
    
#if defined(DEBUG_UPDATE)
    std::cout << "Update error: " << errorMessage.toStdString() << std::endl;
#endif
    if (btnUpdate)
	    btnUpdate->setEnabled(true);
}

void UpdaterWindow::setCss(QString style)
{
	this->setStyleSheet(style);
}
