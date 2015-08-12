#include "stdafx.h"
#include "scene.h"
#include <stdlib.h>
#include <cassert>
using namespace std;
Scene::Scene(string name) :_name(name), id(0)
{

}

Scene::~Scene()
{

}

void Scene::init()
{
	;
}

void Scene::update(float dt)
{
	throw 0;
}

bool Scene::sceneMouseEvent(mouseEvent _event)
{
	return false;
}

void Scene::setname(string name)
{
	_name = name;
}

string Scene::getname()
{
	return _name;
}

int Scene::addchild(Sprite* child)
{
	id++;
	child->setid(id);
	_sprites.push_back(child);
	return id;
}


void Scene::removeChildByID(int id)
{
	vector<Sprite*>::iterator it = _sprites.begin();
	while ( it != _sprites.end())
	{
		if ((*it)->getid() == id)
		{
			_sprites.erase(it);
		}
		else
		{
			it++;
		}
	}
}

Sprite* Scene::getChildByID(int id)
{
	if (_sprites.size())
	{
		for (vector<Sprite*>::iterator it = _sprites.begin(); it != _sprites.end(); it++)
		{
			if (id == (*it)->getid())//�����ҵ��˶�ӦID�ľ���
				return (*it);
		}
	}
	throw 0;//û�ҵ���������Ϊ�գ��׳��쳣
}

void Scene::handleMouseEvent(Point pt)
{
	;
}

vector<Sprite*>& Scene::getSpriteList()
{
	return _sprites;
}

void Scene::registMouseEvent(Sprite* _sprite)
{
	_mouseEventList.push_back(_sprite);
}

void Scene::processMouseEvent(mouseEvent _event)
{
	if (sceneMouseEvent(_event))
	{
		return;
	}
	if (_mouseEventList.size())
	{
		for (vector<Sprite*>::iterator it = _mouseEventList.begin(); it != _mouseEventList.end(); it++)
		{
			if ((*it)->mouseEventCallBack(_event))
				return;                                 //��mouseEventCallBack����ֵΪ��ʱ���ж���Ϣ�Ѿ����������
		}
	}
}

void Scene::onKeyPressedCallback(int key)
{
	assert(key < 256);
	_keys[key]=1;
}

void Scene::clearKeys()
{
	for (int i = 0; i < 256; i++)
	{
		_keys[i] = 0;
	}
}