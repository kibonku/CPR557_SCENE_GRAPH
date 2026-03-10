#ifndef __MY_APPLICATION_H__
#define __MY_APPLICATION_H__

#include "my_window.h"
#include "my_device.h"
#include "my_renderer.h"
#include "my_game_object.h"

#include <memory>
#include <vector>

class MyApplication 
{
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	enum MyAppKeyMap
	{
		KEY_NONE = 0,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_UP,
		KEY_DOWN
	};

	MyApplication();

	void run();

	// Interaction
	void switchProjectionMatrix();
	void switchInteractionMode();
	void handleMovement(MyAppKeyMap key);
	void printSceneGraph();
	void traverseNext();

private:
	void _loadGameObjects();
	void _moveGameObject(MySceneGraphNode *pobj);
	void _printControls();

	MyWindow                           m_myWindow{ WIDTH, HEIGHT, "Assignment 3" };
	MyDevice                           m_myDevice{ m_myWindow };
	MyRenderer                         m_myRenderer{ m_myWindow, m_myDevice };

	std::shared_ptr<MySceneGraphNode>  m_pMySeceneGraphRoot;               // Scene graph root node pointer
	bool                               m_bPerspectiveProjection;           // Switch between orthographic and perspective camera
	bool                               m_bMoveCamera;                      // Move camera or move game object(s)

	MySceneGraphNode*                  m_pCurrentSceneGraphNode = nullptr; // Pointer to the current node in the scene graph
};

#endif