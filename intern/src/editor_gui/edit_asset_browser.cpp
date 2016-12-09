
#include "edit_asset_browser.h"

namespace Edit
{
    CAssetBrowser::CAssetBrowser(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_pDirectoryModel(0)
        , m_pFileModel     (0)
        , m_RootPath       ("../assets/")
        , m_CurrentPath    ("../assets/")
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Prepare model
        // -----------------------------------------------------------------------------
        m_pDirectoryModel = new QFileSystemModel(this);

        m_pFileModel = new QFileSystemModel(this);
    }

    // -----------------------------------------------------------------------------

    CAssetBrowser::~CAssetBrowser() 
    {
        delete m_pDirectoryModel;
        delete m_pFileModel;

        m_pDirectoryModel = 0;
        m_pFileModel      = 0;
    }

    // -----------------------------------------------------------------------------

    void CAssetBrowser::Reset()
    {
        // -----------------------------------------------------------------------------
        // Setup models
        // -----------------------------------------------------------------------------
        m_pDirectoryTreeView->setModel(m_pDirectoryModel);

        m_pDirectoryModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

        m_pDirectoryModel->setRootPath(m_RootPath);

        m_pDirectoryTreeView->setRootIndex(m_pDirectoryModel->index(m_RootPath));

        // -----------------------------------------------------------------------------

        m_pFileTreeView->setModel(m_pFileModel);

        m_pFileModel->setRootPath(m_CurrentPath);

        m_pFileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);

        m_pFileTreeView->setRootIndex(m_pFileModel->index(m_CurrentPath));
    }

    // -----------------------------------------------------------------------------

    void CAssetBrowser::directoryClicked(const QModelIndex& _rModelIndex)
    {
        m_CurrentPath = m_pDirectoryModel->fileInfo(_rModelIndex).absoluteFilePath();

        m_pFileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);

        m_pFileModel->setRootPath(m_CurrentPath);

        m_pFileTreeView->setRootIndex(m_pFileModel->index(m_CurrentPath));
    }
} // namespace Edit