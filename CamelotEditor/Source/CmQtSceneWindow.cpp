#include "CmQtSceneWindow.h"
#include "QtWidgets/QFrame"

namespace CamelotEditor
{
	QtSceneWindow::QtSceneWindow(QWidget* parent)
		:QtEditorWindow(parent, "Scene", tr("Scene"))
	{
		QFrame* frame = new QFrame(this);
	}

	QtSceneWindow::~QtSceneWindow() { }
}