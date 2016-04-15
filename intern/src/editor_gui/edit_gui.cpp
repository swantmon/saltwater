
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "editor_gui/edit_gui.h"
#include "editor_gui/edit_mainwindow.h"

using namespace Edit;

namespace 
{
    class CEditorGui : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEditorGui)

    public:

        CEditorGui();
        ~CEditorGui();

    public:

        void Create(int& _rArgc, char** _ppArgv);
        void Destroy();

        void Setup(int _Width, int _Height);

        void Show();

        void ProcessEvents();

        void* GetEditorWindowHandle();

    private:

        QApplication* m_pApplication;
        CMainWindow*  m_pMainWindow;

    };
} // namespace 

namespace
{
    CEditorGui::CEditorGui()
    {

    }
    
    // -----------------------------------------------------------------------------

    CEditorGui::~CEditorGui()
    {

    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Create(int& _rArgc, char** _ppArgv)
    {
        m_pApplication = new QApplication(_rArgc, _ppArgv);

        m_pMainWindow = new CMainWindow();
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Destroy()
    {
        delete m_pMainWindow;
        delete m_pApplication;

        m_pMainWindow  = 0;
        m_pApplication = 0;
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Setup(int _Width, int _Height)
    {
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Show()
    {
        m_pMainWindow->show();
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessEvents()
    {
        m_pApplication->processEvents();
    }

    // -----------------------------------------------------------------------------

    void* CEditorGui::GetEditorWindowHandle()
    {
        return m_pMainWindow->GetEditorWindowHandle();
    }
} // namespace 

namespace Edit
{
namespace GUI
{
    void Create(int& _rArgc, char** _ppArgv)
    {
        CEditorGui::GetInstance().Create(_rArgc, _ppArgv);
    }
    
    // -----------------------------------------------------------------------------

    void Destroy()
    {
        CEditorGui::GetInstance().Destroy();
    }

    // -----------------------------------------------------------------------------

    void Setup(int _Width, int _Height)
    {
        CEditorGui::GetInstance().Setup(_Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void Show()
    {
        CEditorGui::GetInstance().Show();
    }

    // -----------------------------------------------------------------------------

    void ProcessEvents()
    {
        CEditorGui::GetInstance().ProcessEvents();
    }

    // -----------------------------------------------------------------------------

    void* GetEditorWindowHandle()
    {
        return CEditorGui::GetInstance().GetEditorWindowHandle();
    }
} // namespace GUI
} // namespace Edit