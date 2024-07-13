/* Copyright (c) 2019-2024 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "die_script.h"

bool sort_signature_prio(const DiE_ScriptEngine::SIGNATURE_RECORD &sr1, const DiE_ScriptEngine::SIGNATURE_RECORD &sr2)
{
    if ((sr1.sName == "_init") && (sr2.sName == "_init")) {
        return false;
    }

    if (sr1.sName == "_init") {
        return true;
    } else if (sr2.sName == "_init") {
        return false;
    }

    qint32 nPos1 = sr1.sName.count(".");
    qint32 nPos2 = sr2.sName.count(".");

    if ((nPos1 > 1) && (nPos2 > 1)) {
        QString sPrio1 = sr1.sName.section(".", nPos1 - 1, nPos1 - 1);
        QString sPrio2 = sr2.sName.section(".", nPos2 - 1, nPos2 - 1);

        if ((sPrio1 != "") && (sPrio2 != "")) {
            if (sPrio1 > sPrio2) {
                return false;
            } else if (sPrio1 < sPrio2) {
                return true;
            } else if (sPrio1 == sPrio2) {
                return (sr1.sName.section(".", nPos1 - 2, nPos1 - 2) < sr2.sName.section(".", nPos2 - 2, nPos2 - 2));
            }
        }
    }

    return (sr1.sName < sr2.sName);
}

bool sort_signature_name(const DiE_ScriptEngine::SIGNATURE_RECORD &sr1, const DiE_ScriptEngine::SIGNATURE_RECORD &sr2)
{
    if ((sr1.sName == "_init") && (sr2.sName == "_init")) {
        return false;
    }

    if (sr1.sName == "_init") {
        return true;
    } else if (sr2.sName == "_init") {
        return false;
    }

    return (sr1.sName < sr2.sName);
}

DiE_Script::DiE_Script(QObject *pParent) : XScanEngine(pParent)
{
    g_databaseType = DBT_UNKNOWN;
    g_bIsErrorLogEnable = true;
    g_bIsWarningLogEnable = true;
    g_bIsInfoLogEnable = true;
#ifdef QT_SCRIPTTOOLS_LIB
    g_pDebugger = nullptr;
#endif
}

DiE_ScriptEngine::SIGNATURE_RECORD DiE_Script::_loadSignatureRecord(const QFileInfo &fileInfo, XBinary::FT fileType)
{
    DiE_ScriptEngine::SIGNATURE_RECORD result = {};

    result.fileType = fileType;
    result.sName = fileInfo.fileName();
    result.sText = XBinary::readFile(fileInfo.absoluteFilePath());
    result.sFilePath = fileInfo.absoluteFilePath();

    return result;
}

QList<DiE_ScriptEngine::SIGNATURE_RECORD> DiE_Script::_loadDatabasePath(const QString &sDatabasePath, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    QList<DiE_ScriptEngine::SIGNATURE_RECORD> listResult;

    QDir dir(sDatabasePath);

    QFileInfoList eil = dir.entryInfoList();

    qint32 nNumberOfFiles = eil.count();

    for (qint32 i = 0; (i < nNumberOfFiles) && (!(pPdStruct->bIsStop)); i++) {
        if (eil.at(i).isFile()) {
            QString sSuffix = eil.at(i).suffix().toLower();

            if ((sSuffix == "sg") || (sSuffix == "")) {
                DiE_ScriptEngine::SIGNATURE_RECORD record = _loadSignatureRecord(eil.at(i), fileType);
                listResult.append(record);
            }
        }
    }

    std::sort(listResult.begin(), listResult.end(), sort_signature_prio);

    return listResult;
}

QList<DiE_ScriptEngine::SIGNATURE_RECORD> DiE_Script::_loadDatabaseFromZip(XZip *pZip, QList<XArchive::RECORD> *pListRecords, const QString &sPrefix,
                                                                           XBinary::FT fileType)
{
    QList<DiE_ScriptEngine::SIGNATURE_RECORD> listResult;

    qint32 nNumberOfRecords = pListRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XArchive::RECORD zipRecord = pListRecords->at(i);

        if (((sPrefix == "") && (!zipRecord.sFileName.contains("/"))) ||
            ((zipRecord.sFileName.contains("/")) && (zipRecord.sFileName.section("/", 0, 0) == sPrefix) && (zipRecord.sFileName.section("/", 1, 1) != ""))) {
            QFileInfo fi(zipRecord.sFileName);

            DiE_ScriptEngine::SIGNATURE_RECORD record = {};

            record.fileType = fileType;
            record.sName = fi.fileName();
            record.sText = pZip->decompress(&zipRecord, nullptr);
            record.sFilePath = zipRecord.sFileName;
            record.bReadOnly = true;

            listResult.append(record);
        }
    }

    return listResult;
}

void DiE_Script::processDetect(SCANID *pScanID, XScanEngine::SCAN_RESULT *pScanResult, QIODevice *pDevice, const SCANID &parentId, XBinary::FT fileType,
                               XScanEngine::SCAN_OPTIONS *pOptions, const QString &sSignatureFilePath, bool bAddUnknown, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QString sDetectFunction = "detect";

    if (pOptions->sDetectFunction != "") {
        sDetectFunction = pOptions->sDetectFunction;
    }

    g_bIsErrorLogEnable = true;
    g_bIsWarningLogEnable = false;
    g_bIsInfoLogEnable = false;

    if ((pOptions->bResultAsCSV) || (pOptions->bResultAsJSON) || (pOptions->bResultAsTSV) || (pOptions->bResultAsXML)) {
        g_bIsErrorLogEnable = false;
        g_bIsWarningLogEnable = false;
        g_bIsInfoLogEnable = false;
    }

    if (pOptions->bIsProfiling) {
        g_bIsInfoLogEnable = true;
        g_bIsWarningLogEnable = true;
    }

    QList<DiE_ScriptEngine::SCAN_STRUCT> listRecords;
    XScanEngine::SCANID resultId = {};

    XBinary::_MEMORY_MAP memoryMap = XFormats::getMemoryMap(fileType, XBinary::MAPMODE_UNKNOWN, pDevice, false, -1, pPdStruct);

    resultId.fileType = fileType;
    resultId.sUuid = XBinary::generateUUID();
    resultId.sArch = memoryMap.sArch;
    resultId.mode = memoryMap.mode;
    resultId.endian = memoryMap.endian;
    resultId.sType = memoryMap.sType;
    resultId.nOffset = XIODevice::getInitLocation(pDevice);
    resultId.nSize = pDevice->size();
    resultId.filePart = XBinary::FILEPART_HEADER;

    qint32 nNumberOfSignatures = g_listSignatures.count();

    DiE_ScriptEngine::SIGNATURE_RECORD srGlobalInit = {};
    DiE_ScriptEngine::SIGNATURE_RECORD srInit = {};

    bool bGlobalInit = false;
    bool bInit = false;

    for (qint32 i = 0; (i < nNumberOfSignatures) && (!(pPdStruct->bIsStop)); i++) {
        if (g_listSignatures.at(i).sName == "_init") {
            if (g_listSignatures.at(i).fileType == XBinary::FT_UNKNOWN) {
                srGlobalInit = g_listSignatures.at(i);
                bGlobalInit = true;
            }

            if (XBinary::checkFileType(g_listSignatures.at(i).fileType, fileType)) {
                srInit = g_listSignatures.at(i);
                bInit = true;
            }
        }

        if (bGlobalInit && bInit) {
            break;
        }
    }

    Binary_Script::OPTIONS _options = {};
    _options.bIsDeepScan = pOptions->bIsDeepScan;
    _options.bIsHeuristicScan = pOptions->bIsHeuristicScan;
    _options.bIsVerbose = pOptions->bIsVerbose;
    _options.bIsProfiling = pOptions->bIsProfiling;

    DiE_ScriptEngine scriptEngine(&g_listSignatures, &listRecords, pDevice, fileType, &_options, pPdStruct);

    connect(&scriptEngine, SIGNAL(errorMessage(QString)), this, SLOT(_errorMessage(QString)));
    connect(&scriptEngine, SIGNAL(warningMessage(QString)), this, SLOT(_warningMessage(QString)));
    connect(&scriptEngine, SIGNAL(infoMessage(QString)), this, SLOT(_infoMessage(QString)));

#ifdef QT_SCRIPTTOOLS_LIB
    if (g_pDebugger) {
        g_pDebugger->attachTo(&scriptEngine);
    }
#endif

    if (nNumberOfSignatures) {
        if (bGlobalInit) {
            _handleError(&scriptEngine, scriptEngine.evaluate(srGlobalInit.sText, srGlobalInit.sFilePath), &srGlobalInit, pScanResult);
        }

        if (bInit) {
            _handleError(&scriptEngine, scriptEngine.evaluate(srInit.sText, srInit.sFilePath), &srInit, pScanResult);
        }
    }

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nNumberOfSignatures);

    for (qint32 i = 0; (i < nNumberOfSignatures) && (!(pPdStruct->bIsStop)); i++) {
        DiE_ScriptEngine::SIGNATURE_RECORD signatureRecord = g_listSignatures.at(i);

        XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, signatureRecord.sName);

        bool bExec = false;

        if (XBinary::checkFileType(signatureRecord.fileType, fileType)) {
            bExec = true;
        }

        if (pOptions->sSignatureName != "") {
            if (pOptions->sSignatureName != signatureRecord.sName) {
                bExec = false;
            }
        }

        if (!pOptions->bIsDeepScan) {
            QString sPrefix = signatureRecord.sName.section(".", 0, 0).toUpper();

            if ((sPrefix == "DS") || (sPrefix == "EP")) {
                bExec = false;
            }
        }

        if (!pOptions->bIsHeuristicScan) {
            QString sPrefix = signatureRecord.sName.section(".", 0, 0).toUpper();

            if (sPrefix == "HEUR") {
                bExec = false;
            }
        }

        if (sSignatureFilePath != "")  // TODO Check!
        {
            bExec = (sSignatureFilePath == signatureRecord.sFilePath);
        }

        if (bExec) {
            // scriptEngine.clearListLocalResult();
            if (pOptions->bIsProfiling) {
                emit warningMessage(QString("%1").arg(signatureRecord.sName));
            }

            QElapsedTimer *pElapsedTimer = nullptr;

            if ((pOptions->bShowScanTime) || (pOptions->bIsProfiling)) {
                pElapsedTimer = new QElapsedTimer;
                pElapsedTimer->start();
            }

            XSCRIPTVALUE script = scriptEngine.evaluateEx(parentId, resultId, signatureRecord.sText, signatureRecord.sName, signatureRecord.sFilePath);

            if (_handleError(&scriptEngine, script, &signatureRecord, pScanResult)) {
#ifdef QT_SCRIPTTOOLS_LIB
                if (g_pDebugger) {
                    g_pDebugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
                }
#endif
                XSCRIPTVALUE _scriptValue = scriptEngine.globalObject().property(sDetectFunction);

                if (_handleError(&scriptEngine, _scriptValue, &signatureRecord, pScanResult)) {
                    XSCRIPTVALUELIST valuelist;

                    if (sDetectFunction == "detect") {
                        valuelist << pOptions->bShowType << pOptions->bShowVersion << pOptions->bShowOptions;
                    }

#ifdef QT_SCRIPT_LIB
                    QScriptValue result = _scriptValue.call(script, valuelist);
#else
                    QJSValue result = _scriptValue.callWithInstance(script, valuelist);
#endif

                    if (_handleError(&scriptEngine, result, &signatureRecord, pScanResult)) {
                        // // TODO getResult
                        // QString sResult = result.toString();

                        // QList<DiE_ScriptEngine::RESULT> listLocalResult = scriptEngine.getListLocalResult();
                        // qint32 nNumberOfDetects = listLocalResult.count();

                        // if ((nNumberOfDetects == 0) && (sResult != "")) {
                        //     listLocalResult.append(DiE_ScriptEngine::stringToResult(sResult, pOptions->bShowType, pOptions->bShowVersion, pOptions->bShowOptions));
                        // }

                        // for (qint32 j = 0; j < nNumberOfDetects; j++) {
                        //     DiE_ScriptEngine::SCAN_STRUCT ssRecord = {};

                        //     // TODO IDs
                        //     ssRecord.id = resultId;
                        //     ssRecord.parentId = parentId;

                        //     ssRecord.sSignature = signatureRecord.sName;
                        //     ssRecord.sSignatureFileName = signatureRecord.sFilePath;
                        //     ssRecord.sType = listLocalResult.at(j).sType;
                        //     ssRecord.sName = listLocalResult.at(j).sName;
                        //     ssRecord.sVersion = listLocalResult.at(j).sVersion;
                        //     ssRecord.sOptions = listLocalResult.at(j).sOptions;
                        //     ssRecord.sFullString = QString("%1: %2(%3)[%4]").arg(ssRecord.sType, ssRecord.sName, ssRecord.sVersion, ssRecord.sOptions);
                        //     ssRecord.sResult = QString("%1(%2)[%3]").arg(ssRecord.sName, ssRecord.sVersion, ssRecord.sOptions);

                        //     listRecords.append(ssRecord);
                        // }
                    }
                }
            }

            if (pElapsedTimer) {
                qint64 nElapsedTime = pElapsedTimer->elapsed();
                delete pElapsedTimer;

                if (pOptions->bShowScanTime) {
                    XScanEngine::DEBUG_RECORD debugRecord = {};
                    debugRecord.sScript = signatureRecord.sName;
                    debugRecord.nElapsedTime = nElapsedTime;

                    // #ifdef QT_DEBUG
                    //                     qDebug("%s: %lld msec", debugRecord.sScript.toLatin1().data(), debugRecord.nElapsedTime);
                    // #endif
                    pScanResult->listDebugRecords.append(debugRecord);
                }

                if (pOptions->bIsProfiling) {
                    emit warningMessage(QString("%1: [%2 ms]").arg(signatureRecord.sName, QString::number(nElapsedTime)));
                }
            }
        }

        XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndex);
    }

    if (bAddUnknown) {
        if (listRecords.count() == 0) {
            DiE_ScriptEngine::SCAN_STRUCT ssRecord = {};

            ssRecord.id = resultId;
            ssRecord.parentId = parentId;

            listRecords.append(ssRecord);
        }
    }

    QList<XScanEngine::SCANSTRUCT> listScanStruct = convert(&listRecords);

    pScanResult->listRecords.append(listScanStruct);

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (pScanID) {
        *pScanID = resultId;
    }
}

bool DiE_Script::_handleError(DiE_ScriptEngine *pScriptEngine, XSCRIPTVALUE scriptValue, DiE_ScriptEngine::SIGNATURE_RECORD *pSignatureRecord,
                              XScanEngine::SCAN_RESULT *pScanResult)
{
    bool bResult = false;

    QString sErrorString;
    if (pScriptEngine->handleError(scriptValue, &sErrorString)) {
        bResult = true;
    } else {
        XScanEngine::ERROR_RECORD errorRecord = {};
        errorRecord.sScript = pSignatureRecord->sName;
        errorRecord.sErrorString = sErrorString;

        pScanResult->listErrors.append(errorRecord);
    }

    return bResult;
}

void DiE_Script::_processDetect(SCANID *pScanID, SCAN_RESULT *pScanResult, QIODevice *pDevice, const SCANID &parentId, XBinary::FT fileType, SCAN_OPTIONS *pOptions,
                                bool bAddUnknown, XBinary::PDSTRUCT *pPdStruct)
{
    processDetect(pScanID, pScanResult, pDevice, parentId, fileType, pOptions, "", bAddUnknown, pPdStruct);
}

void DiE_Script::_errorMessage(const QString &sErrorMessage)
{
    if (g_bIsErrorLogEnable) {
        emit errorMessage(sErrorMessage);
    }
}

void DiE_Script::_warningMessage(const QString &sWarningMessage)
{
    if (g_bIsWarningLogEnable) {
        emit warningMessage(sWarningMessage);
    }
}

void DiE_Script::_infoMessage(const QString &sInfoMessage)
{
    if (g_bIsInfoLogEnable) {
        emit infoMessage(sInfoMessage);
    }
}

bool DiE_Script::loadDatabase(const QString &sDatabasePath, bool bInit, XBinary::PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    QElapsedTimer *pElapsedTimer = new QElapsedTimer;
    pElapsedTimer->start();
#endif
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    g_databaseType = DBT_UNKNOWN;

    if (bInit) {
        g_listSignatures.clear();
    }

    QString _sDatabasePath = XBinary::convertPathName(sDatabasePath);

    if (XBinary::isFileExists(_sDatabasePath))  // Load from zip
    {
        QFile file;
        file.setFileName(_sDatabasePath);

        if (file.open(QIODevice::ReadOnly)) {
            XZip zip(&file);

            if (zip.isValid()) {
                QList<XArchive::RECORD> listRecords = zip.getRecords(-1, pPdStruct);  // TODO Check

                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "", XBinary::FT_UNKNOWN));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "Binary", XBinary::FT_BINARY));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "COM", XBinary::FT_COM));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "Archive", XBinary::FT_ARCHIVE));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "ZIP", XBinary::FT_ZIP));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "JAR", XBinary::FT_JAR));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "APK", XBinary::FT_APK));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "IPA", XBinary::FT_IPA));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "NPM", XBinary::FT_NPM));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "DEB", XBinary::FT_DEB));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "DEX", XBinary::FT_DEX));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "MSDOS", XBinary::FT_MSDOS));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "LE", XBinary::FT_LE));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "LX", XBinary::FT_LX));  // TODO Check
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "NE", XBinary::FT_NE));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "PE", XBinary::FT_PE));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "ELF", XBinary::FT_ELF));
                g_listSignatures.append(_loadDatabaseFromZip(&zip, &listRecords, "MACH", XBinary::FT_MACHO));

                g_databaseType = DBT_COMPRESSED;
            }

            file.close();
        }
    } else if (XBinary::isDirectoryExists(_sDatabasePath)) {
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath, XBinary::FT_UNKNOWN, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "Binary", XBinary::FT_BINARY, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "COM", XBinary::FT_COM, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "Archive", XBinary::FT_ARCHIVE, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "ZIP", XBinary::FT_ZIP, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "JAR", XBinary::FT_JAR, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "APK", XBinary::FT_APK, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "IPA", XBinary::FT_IPA, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "NPM", XBinary::FT_NPM, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "DEB", XBinary::FT_NPM, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "DEX", XBinary::FT_DEX, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "MSDOS", XBinary::FT_MSDOS, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "LE", XBinary::FT_LE, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "LX", XBinary::FT_LX, pPdStruct));  // TODO Check
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "NE", XBinary::FT_NE, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "PE", XBinary::FT_PE, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "ELF", XBinary::FT_ELF, pPdStruct));
        g_listSignatures.append(_loadDatabasePath(_sDatabasePath + QDir::separator() + "MACH", XBinary::FT_MACHO, pPdStruct));

        g_databaseType = DBT_FOLDER;
    } else {
        QString sErrorString = QString("%1: %2").arg(tr("Cannot load database"), sDatabasePath);

        if (bInit) {
            _errorMessage(sErrorString);
        } else {
            _warningMessage(sErrorString);
        }
    }

#ifdef QT_DEBUG
    qDebug("DiE_Script::loadDatabase: %d ms", pElapsedTimer->elapsed());
#endif

    return g_listSignatures.count();
}

QList<DiE_Script::SIGNATURE_STATE> DiE_Script::getSignatureStates()
{
    QList<SIGNATURE_STATE> listResult;

    QList<XBinary::FT> listFT;

    listFT.append(XBinary::FT_BINARY);
    listFT.append(XBinary::FT_COM);
    listFT.append(XBinary::FT_MSDOS);
    listFT.append(XBinary::FT_NE);
    listFT.append(XBinary::FT_LE);
    listFT.append(XBinary::FT_LX);
    listFT.append(XBinary::FT_PE);
    listFT.append(XBinary::FT_ELF);
    listFT.append(XBinary::FT_MACHO);
    listFT.append(XBinary::FT_ZIP);
    listFT.append(XBinary::FT_JAR);
    listFT.append(XBinary::FT_APK);
    listFT.append(XBinary::FT_IPA);
    listFT.append(XBinary::FT_DEX);
    listFT.append(XBinary::FT_NPM);

    qint32 nNumberOfFileTypes = listFT.count();

    for (qint32 i = 0; i < nNumberOfFileTypes; i++) {
        SIGNATURE_STATE state = {};
        state.fileType = listFT.at(i);
        state.nNumberOfSignatures = getNumberOfSignatures(state.fileType);

        listResult.append(state);
    }

    return listResult;
}

qint32 DiE_Script::getNumberOfSignatures(XBinary::FT fileType)
{
    qint32 nResult = 0;

    qint32 nNumberOfSignatures = g_listSignatures.count();

    for (qint32 i = 0; (i < nNumberOfSignatures); i++) {
        if ((g_listSignatures.at(i).sName != "_init") && (XBinary::checkFileType(g_listSignatures.at(i).fileType, fileType))) {
            nResult++;
        }
    }

    return nResult;
}

QList<DiE_ScriptEngine::SIGNATURE_RECORD> *DiE_Script::getSignatures()
{
    return &g_listSignatures;
}

DiE_ScriptEngine::SIGNATURE_RECORD DiE_Script::getSignatureByFilePath(const QString &sSignatureFilePath)
{
    DiE_ScriptEngine::SIGNATURE_RECORD result = {};

    qint32 nNumberOfSignatures = g_listSignatures.count();

    for (qint32 i = 0; i < nNumberOfSignatures; i++) {
        if (g_listSignatures.at(i).sFilePath == sSignatureFilePath) {
            result = g_listSignatures.at(i);

            break;
        }
    }

    return result;
}

bool DiE_Script::updateSignature(const QString &sSignatureFilePath, const QString &sText)
{
    bool bResult = false;

    qint32 nNumberOfSignatures = g_listSignatures.count();

    for (qint32 i = 0; i < nNumberOfSignatures; i++) {
        if (g_listSignatures.at(i).sFilePath == sSignatureFilePath) {
            if (XBinary::writeToFile(sSignatureFilePath, QByteArray().append(sText.toUtf8()))) {
                g_listSignatures[i].sText = sText;
                bResult = true;
            }

            break;
        }
    }

    return bResult;
}

DiE_Script::STATS DiE_Script::getStats()
{
    STATS result = {};

    qint32 nNumberOfSignatures = g_listSignatures.count();

    for (qint32 i = 0; i < nNumberOfSignatures; i++) {
        QString sText = g_listSignatures.at(i).sText;

        QString sType = XBinary::regExp("init\\(\"(.*?)\",", sText, 1);

        if (sType != "") {
            result.mapTypes.insert(sType, result.mapTypes.value(sType, 0) + 1);
        }
    }

    return result;
}

DiE_Script::DBT DiE_Script::getDatabaseType()
{
    return g_databaseType;
}

bool DiE_Script::isSignaturesPresent(XBinary::FT fileType)
{
    bool bResult = false;

    qint32 nNumberOfSignatures = g_listSignatures.count();

    for (qint32 i = 0; i < nNumberOfSignatures; i++) {
        if (g_listSignatures.at(i).fileType == fileType) {
            bResult = true;

            break;
        }
    }

    return bResult;
}

QString DiE_Script::getErrorsString(XScanEngine::SCAN_RESULT *pScanResult)
{
    QString sResult;

    qint32 nNumberOfErrors = pScanResult->listErrors.count();

    for (qint32 i = 0; i < nNumberOfErrors; i++) {
        sResult += QString("%1: %2\n").arg(pScanResult->listErrors.at(i).sScript, pScanResult->listErrors.at(i).sErrorString);
    }

    return sResult;
}

QList<QString> DiE_Script::getErrorsAndWarningsStringList(XScanEngine::SCAN_RESULT *pScanResult)
{
    QList<QString> listResult;

    qint32 nNumberOfErrors = pScanResult->listErrors.count();

    for (qint32 i = 0; i < nNumberOfErrors; i++) {
        listResult.append(QString("%1: %2").arg(pScanResult->listErrors.at(i).sScript, pScanResult->listErrors.at(i).sErrorString));
    }

    return listResult;
}

QList<XScanEngine::SCANSTRUCT> DiE_Script::convert(QList<DiE_ScriptEngine::SCAN_STRUCT> *pListScanStructs)
{
    QList<XScanEngine::SCANSTRUCT> listResult;

    qint32 nNumberOfRecords = pListScanStructs->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XScanEngine::SCANSTRUCT record = {};

        record.bIsHeuristic = pListScanStructs->at(i).bIsHeuristic;
        record.id = pListScanStructs->at(i).id;
        record.parentId = pListScanStructs->at(i).parentId;
        record.sType = pListScanStructs->at(i).sType;
        record.sName = pListScanStructs->at(i).sName;
        record.sVersion = pListScanStructs->at(i).sVersion;
        record.sInfo = pListScanStructs->at(i).sOptions;
        record.varInfo = pListScanStructs->at(i).sSignature;
        record.varInfo2 = pListScanStructs->at(i).sSignatureFileName;
        // record.sResult = pListScanStructs->at(i).sResult;

        record.globalColor = typeToColor(record.sType);
        record.nPrio = typeToPrio(record.sType);
        record.bIsProtection = isProtection(record.sType);
        record.sType = translateType(record.sType);

        listResult.append(record);
    }

    sortRecords(&listResult);

    return listResult;
}

#ifdef QT_SCRIPTTOOLS_LIB
void DiE_Script::setDebugger(QScriptEngineDebugger *pDebugger)
{
    this->g_pDebugger = pDebugger;
}
#endif

#ifdef QT_SCRIPTTOOLS_LIB
void DiE_Script::removeDebugger()
{
    this->g_pDebugger = nullptr;
}
#endif
