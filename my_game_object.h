#ifndef __MY_GAMEOBJECT_H__
#define __MY_GAMEOBJECT_H__

#include "my_model.h"
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <vector>
#include <array>

struct TransformComponent
{
	glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };

	glm::mat4 mat4() 
	{	
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} };
	}
};

class MyGameObject
{
public:
	using id_t = unsigned int;

	static MyGameObject createGameObject(std::string name)
	{
		static id_t currentID = 0;
		return MyGameObject(currentID++, name);
	}

	MyGameObject(const MyGameObject&) = delete;
	MyGameObject& operator=(const MyGameObject&) = delete;
	MyGameObject(MyGameObject&&) = default;
	MyGameObject& operator=(MyGameObject&&) = default;

	std::string              getName() const { return m_sName; }
	id_t                     getID()   const { return m_iID; }
	std::shared_ptr<MyModel> model{};
	glm::vec3                color{};
	TransformComponent       transform{};

protected:
	MyGameObject(id_t objID, std::string name) : m_iID{ objID }, m_sName{ name } {}
	id_t m_iID;
	std::string m_sName;
};

class MySceneGraphNode : public MyGameObject
{
public:

    static MySceneGraphNode createGameObject(std::string name)
	{
		static id_t currentID = 0;
		return MySceneGraphNode(name, currentID++);
	}
	
	MySceneGraphNode(std::string name, int id) : MyGameObject(id, name)
	{
	}

	void addChild(std::shared_ptr<MySceneGraphNode>& obj);
	void printSceneGraph();
	MySceneGraphNode* traverseNext(MySceneGraphNode* pCurrentNode);

	// Public accessors used by renderer and application
	bool isCurrent() const { return m_bIsCurrent; }
	const std::vector<std::shared_ptr<MySceneGraphNode>>& getChildren() const { return m_vMyChildren; }

private:
	void _printSceneGraph(int indent);
	void _collectNodes(std::vector<MySceneGraphNode*>& nodes);

	std::vector<std::shared_ptr<MySceneGraphNode>> m_vMyChildren;
	std::shared_ptr<MySceneGraphNode> m_pMyParent;

	bool m_bIsCurrent = false;
};

#endif
