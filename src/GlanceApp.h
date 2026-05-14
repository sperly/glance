#ifndef GLANCE_APP_H
#define GLANCE_APP_H

#include <wx/wx.h>

class MainFrame;

class GlanceApp : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;
    
private:
    MainFrame* m_mainFrame;
};

#endif // GLANCE_APP_H