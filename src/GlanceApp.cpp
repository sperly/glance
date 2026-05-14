#include "GlanceApp.h"
#include "MainFrame.h"

bool GlanceApp::OnInit()
{
    m_mainFrame = new MainFrame();
    m_mainFrame->Show(true);
    return true;
}

int GlanceApp::OnExit()
{
    return 0;
}