#include "kdtree.h"
#include "defines.h"
#include "scene.h"
#include "scene_types.h"
#include <iostream>

#include <vector>
#include <stack>
#include <queue>

typedef struct s_kdtreeNode KdTreeNode;
const int XAXIS = 0, YAXIS = 1, ZAXIS = 2;

struct s_kdtreeNode
{
	bool leaf;				  //! is this node a leaf ?
	int axis;				  //! axis index of the split, if not leaf
	float split;			  //!position of the split
	int depth;				  //!depth in the tree
	std::vector<int> objects; //! index of objects, if leaf
	KdTreeNode *left;		  //!ptr to left child
	KdTreeNode *right;		  //! ptr to right child
	BoundingBox bounding_box; //! bounding box of the node
};

KdTreeNode *initNode(bool l, int a, int d)
{
	KdTreeNode *ret = new KdTreeNode();
	ret->leaf = l;
	ret->axis = a;
	ret->depth = d;
	ret->left = NULL;
	ret->right = NULL;
	return ret;
}

typedef struct s_stackNode
{
	float tmin;
	float tmax;
	KdTreeNode *node;
} StackNode;

struct s_kdtree
{
	int depthLimit;
	size_t objLimit;
	KdTreeNode *root;

	std::vector<int> outOfTree;
	std::vector<int> inTree;
};

//To help debugging
void printKdTree(KdTree tree)
{
	std::queue<KdTreeNode *> q;
	q.push(tree.root);
	int lastDepth = 0;
	for (; !q.empty();)
	{
		KdTreeNode *node = (KdTreeNode *)q.front();
		q.pop();
		if (node->depth > lastDepth)
		{
			printf("\n%d : ", node->depth);
			lastDepth = node->depth;
		}

		printf("((%.2f,%.2f,%.2f),(%.2f,%.2f,%.2f) %d %d %lu %.2f %d)  ", node->bounding_box.min.x, node->bounding_box.min.y, node->bounding_box.min.z, node->bounding_box.max.x, node->bounding_box.max.y, node->bounding_box.max.z,
			   node->axis, node->depth, node->objects.size(), node->split, node->leaf);
		if (!node->leaf)
		{
			q.push(node->left);
			q.push(node->right);
		}
	}

	printf("\n");
}

void subdivide(Scene *scene, KdTree *tree, KdTreeNode *node);

// Used to determine max depth of the KDTree
int treeDepthLimit(Scene *scene)
{
	return 10;
}

// Used to determine the object limit contained by KDTree leafs
size_t treeObjLimit(Scene *scene)
{
	return scene->objects.size() * 1 / 10 + 1;
}

void updateNodeBoundingBox(KdTreeNode *node, BoundingBox objBox)
{
	node->bounding_box.min = min(node->bounding_box.min, objBox.min);
	node->bounding_box.max = max(node->bounding_box.max, objBox.max);
}

KdTree *initKdTree(Scene *scene)
{

	//!\todo compute scene
	// bbox, store object in outOfTree or inTree depending on type

	KdTree *tree = new KdTree();

	tree->depthLimit = treeDepthLimit(scene);
	tree->objLimit = treeObjLimit(scene);
	tree->root = initNode(true, XAXIS, 0);

	int nbObjects = scene->objects.size();

	for (int i = 0; i < nbObjects; i++)
	{
		Object *obj = scene->objects[i];
		if (obj->geom.type == PLANE)
			tree->outOfTree.push_back(i);
		else
		{
			tree->inTree.push_back(i);
			tree->root->objects.push_back(i);

			BoundingBox objBoundingBox = getObjectBoundingBox(obj);
			if (i == 0)
				tree->root->bounding_box = objBoundingBox;
			else
				updateNodeBoundingBox(tree->root, objBoundingBox);
		}
	}

	subdivide(scene, tree, tree->root);

	return tree;
}

vec3 changeCoordAxis(vec3 v, float newNb, int axis)
{
	v[axis] = newNb;
	return v;
}

//from http://blog.nuclex-games.com/tutorials/collision-detection/static-sphere-vs-aabb/
bool intersectSphereAabb(vec3 sphereCenter, float sphereRadius, vec3 aabbMin, vec3 aabbMax)
{
	vec3 closestPointInAabb = min(max(sphereCenter, aabbMin), aabbMax);
	vec3 seg = closestPointInAabb - sphereCenter;
	float distanceSquared = dot(seg, seg);
	// The AABB and the sphere overlap if the closest point within the rectangle is
	// within the sphere's radius
	return distanceSquared < (sphereRadius * sphereRadius);
}

float surfaceArea(BoundingBox box)
{
	float a = distance(box.min, point3(box.max.x, box.min.y, box.min.z));
	float b = distance(box.min, point3(box.min.x, box.max.y, box.min.z));
	float c = distance(box.min, point3(box.min.x, box.min.y, box.max.z));
	return 2 * a * b + 2 * a * c + 2 * 2 * c;
}

int nextAxisSplit(const KdTreeNode n)
{
	return (n.axis + 1) % 3;
}

float costNode(const KdTreeNode n)
{
	float SA = surfaceArea(n.bounding_box);
	return SA * (float)n.objects.size();
}

/*
* Create a child node of a node. Objects are not added in the child node.
* Child must be free
*/
void buildChild(KdTreeNode parent, KdTreeNode **child, const Scene *scene, float tSplit, bool isLeftChild)
{
	*child = initNode(true, nextAxisSplit(parent), parent.depth + 1);
	if (isLeftChild)
	{
		(*child)->bounding_box.min = parent.bounding_box.min;
		(*child)->bounding_box.max = changeCoordAxis(parent.bounding_box.max, tSplit, parent.axis);
	}
	else
	{
		(*child)->bounding_box.min = changeCoordAxis(parent.bounding_box.min, tSplit, parent.axis);
		(*child)->bounding_box.max = parent.bounding_box.max;
	}
}

void addObjectsToChilrenNodes(const KdTreeNode parent, const Scene *scene, KdTreeNode *nodeLeft, KdTreeNode *nodeRight, const float tSplit)
{
	for (int objIndex : parent.objects)
	{
		Object *obj = scene->objects[objIndex];

		BoundingBox box = getObjectBoundingBox(obj);

		if (box.min[parent.axis] <= tSplit)
			nodeLeft->objects.push_back(objIndex);
		if (box.max[parent.axis] >= tSplit)
			nodeRight->objects.push_back(objIndex);
	}
}

float determineBestSplit(const KdTreeNode nodeToSplit, const Scene *scene)
{
	bool minCostInitialized = false;
	float minCost = 0.f;
	float tMin = 0.f;

	for (int objIndex : nodeToSplit.objects)
	{
		Object *obj = scene->objects[objIndex];

		float boxMinMax[2] = {obj->geom.box.min[nodeToSplit.axis], obj->geom.box.max[nodeToSplit.axis]};

		for (float tSplit : boxMinMax)
		{
			KdTreeNode *nodeLeft = NULL, *nodeRight = NULL;
			buildChild(nodeToSplit, &nodeLeft, scene, tSplit, true);
			buildChild(nodeToSplit, &nodeRight, scene, tSplit, false);

			addObjectsToChilrenNodes(nodeToSplit, scene, nodeLeft, nodeRight, tSplit);

			float cost = costNode(*nodeLeft) + costNode(*nodeRight);
			if (!minCostInitialized || cost < minCost)
			{
				minCostInitialized = true;
				minCost = cost;
				tMin = tSplit;
			}

			delete (nodeLeft);
			delete (nodeRight);
		}
	}

	return tMin;
}

void subdivide(Scene *scene, KdTree *tree, KdTreeNode *node)
{

	if (node->depth < tree->depthLimit && node->objects.size() >= tree->objLimit)
	{
		node->leaf = false;
		node->split = determineBestSplit(*node, scene);
		buildChild(*node, &node->left, scene, node->split, true);
		buildChild(*node, &node->right, scene, node->split, false);
		addObjectsToChilrenNodes(*node, scene, node->left, node->right, node->split);

		subdivide(scene, tree, node->left);
		subdivide(scene, tree, node->right);
	}
}

// from http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
bool intersectAabb(Ray *theRay, vec3 min, vec3 max)
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	vec3 bounds[2] = {min, max};
	tmin = (bounds[theRay->sign[0]].x - theRay->orig.x) * theRay->invdir.x;
	tmax = (bounds[1 - theRay->sign[0]].x - theRay->orig.x) * theRay->invdir.x;
	tymin = (bounds[theRay->sign[1]].y - theRay->orig.y) * theRay->invdir.y;
	tymax = (bounds[1 - theRay->sign[1]].y - theRay->orig.y) * theRay->invdir.y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (bounds[theRay->sign[2]].z - theRay->orig.z) * theRay->invdir.z;
	tzmax = (bounds[1 - theRay->sign[2]].z - theRay->orig.z) * theRay->invdir.z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	if (tmin > theRay->tmin)
		theRay->tmin = tmin;
	if (tmax < theRay->tmax)
		theRay->tmax = tmax;
	return tmin > 0 || tmax > 0;
}

bool traverse(Scene *scene, KdTree *tree, std::stack<StackNode> *stack, StackNode currentNode, Ray *ray, Intersection *intersection)
{
	bool hasIntersection = false;

	KdTreeNode *node = currentNode.node;

	float tmin = currentNode.tmin;
	float tmax = currentNode.tmax;

	for (; !node->leaf;)
	{
		int axis = node->axis;
		float split = node->split;
		if (ray->dir[axis] > 0.f)
		{
			float tsplit = (split - ray->orig[axis]) * ray->invdir[axis];
			if (tsplit <= tmin)
			{ // tsplit <= tmin
				node = node->right;
			}
			else if (tsplit >= tmax)
			{
				node = node->left;
			}
			else
			{
				StackNode sn;
				sn.node = node->right;
				sn.tmin = tsplit;
				sn.tmax = tmax;
				stack->push(sn);

				node = node->left;
				tmax = tsplit;
			}
		}
		else if (ray->dir[axis] < 0.f)
		{
			float tsplit = (split - ray->orig[axis]) * ray->invdir[axis];
			if (tsplit <= tmin)
			{ // tsplit <= tmin
				node = node->left;
			}
			else if (tsplit >= tmax)
			{
				node = node->right;
			}
			else
			{
				StackNode sn;
				sn.node = node->left;
				sn.tmin = tsplit;
				sn.tmax = tmax;
				stack->push(sn);

				node = node->right;
				tmax = tsplit;
			}
		}
		else
		{
			if (ray->orig[axis] < split)
			{
				node = node->left;
			}
			else
			{
				node = node->right;
			}
		}
	}
	ray->tmin = tmin;
	ray->tmax = tmax;

	for (int objIndex : node->objects)
	{
		Object *obj = scene->objects[objIndex];

		switch (obj->geom.type)
		{
		case SPHERE:
			if (intersectSphere(ray, intersection, obj))
				hasIntersection = true;
			break;
		case TRIANGLE:
			if (intersectTriangle(ray, intersection, obj))
				hasIntersection = true;
			break;
		default:
			break;
		}
	}

	return hasIntersection;
}

bool intersectKdTree(Scene *scene, KdTree *tree, Ray *ray, Intersection *intersection)
{
	bool hasIntersection = false;

	//!\todo call vanilla intersection on non kdtree object, then traverse the tree to compute other intersections

	for (int objIndex : tree->outOfTree)
	{
		Object *obj = scene->objects[objIndex];
		switch (obj->geom.type)
		{
		case PLANE:
			hasIntersection = intersectPlane(ray, intersection, obj) || hasIntersection;
			break;
		default:
			break;
		}
	}

	std::stack<StackNode> st = std::stack<StackNode>();

	if (intersectAabb(ray, tree->root->bounding_box.min, tree->root->bounding_box.max))
	{
		StackNode snRoot;
		snRoot.node = tree->root;
		snRoot.tmin = ray->tmin;
		snRoot.tmax = ray->tmax;
		st.push(snRoot);
	}

	bool intersectionTree = false;

	for (; !intersectionTree && !st.empty();)
	{
		StackNode s = (StackNode)st.top();
		st.pop();
		intersectionTree = traverse(scene, tree, &st, s, ray, intersection);
	}

	return hasIntersection || intersectionTree;
}
