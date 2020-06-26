#include <iostream>
#include <fstream>
#include <vector>
//#include <cstdlib>
#include <time.h>
#include <ctime>
//#include <string>
//#include <thread>
#include <math.h>
#include <algorithm>
#include <queue>
#include <map>
#include <set>
//#include <mutex>
#include <windows.h>

#include "Point.h"
#include "Task.h"
#include "Map.h"
#include "Robot.h"
#include "Log.h"

using namespace std;

void ExtendTask(BinNode<vector<int>>* assNode, BinNode<char>* segNode, Task* task, MatrixMap* map, int depth, int obj);
void ExtendAction(BinNode<vector<int>>* assNode, BinNode<char>* segNode, Task* task, MatrixMap* map);
void AssignTaskToRobot(Task* task, vector<Robot*> robot);
void AssignCurrrentTargetToRobot(vector<Robot*> robot, vector<TaskPoint*> allTargets);
vector<vector<int>> IDtoIndex(vector<Robot*> robot);
bool CheckReach(vector<RobotGroup> groups);
vector<RobotGroup> Dock(vector<Robot*> robot, Task* task, vector<int> tID2index, int layer);
vector<int> GetPeers(RobotGroup group, vector<Robot*> robot, Task* task, vector<int> tID2index, int layer);

// 
bool RecordRobotPosition(vector<Robot*> robots);
void Recover(Task* task);

int main() {
	// read map, the origin is in the leftmost top,  x means rows, y means columns
	MatrixMap* world = new MatrixMap();
	world->ReadMap();
	//world.Display();

	// read task, generate assembly tree
	Task* task = new Task();
	task->ReadTask();
	task->GenerateTree();
	//cout << endl << "Depth: " << task->AssemblyTree.depth(task->AssemblyTree.root()) << endl;
	//cout << endl << world->TaskCheck(1, task->AssemblyTree.leaves()[0]->data, 2) << endl; 

	// move the task components, according to the assembly tree
	int depth = task->AssemblyTree.depth(task->AssemblyTree.root());
	for (int i = 0; i < depth-1; i++) { // the leaf layer of assembly tree is not needed for extension
		// extension task
		ExtendTask(task->AssemblyTree.root(), task->SegTree.root(), task, world, 0, i);
		// display
		cout << endl << "Extend step " << i << " : " << endl;
		world->Display("task");
		cout << endl;
		// push currentTargets into allTargets
		task->PushAllTargets();
	}
	RecordLog("Finished to extend the task!");
	// display task positions in steps
	//task->Display("all");
	
	// create the robots
	vector<Robot*> robot;
	for (int i = 0; i < task->robotNum; i++) {
		Robot* temp = new Robot(task->robotNum,task->startPoints[i]->id, task->startPoints[i]->taskPoint, world->RowNum, world->ColNum);
		robot.push_back(temp);
	}
	// assign the task to the closest robots using optimization (or bid)
	AssignTaskToRobot(task, robot);
	// ID to index
	vector<vector<int>> idToIndex = IDtoIndex(robot);
	vector<int> tID2index = idToIndex[0];  // input: task ID  output: robot index
	//vector<int> rID2index = idToIndex[1];  // input: robot ID output: robot index

	// initialize robot groups
	vector<RobotGroup> groups;
	for (int i = 0; i < robot.size(); i++) {  // for each one node 
		vector<Robot*> tempGroup;
		tempGroup.push_back(robot[i]);
		RobotGroup robotGroup(tempGroup);  // robot group
		groups.push_back(robotGroup);
	}

	// robots move
	int stepNum = task->allTargets.size(); // tree depth
	int roboNum = task->allTargets[0].size();
	for (int i = 0; i < stepNum; i++) { // for each one layer
		cout << "In step " << i+1 << " of move:"<< endl;
		task->Display(stepNum - i - 1);
		// update task
		world->UpdateTaskmap(task->allTargets[stepNum - i - 1]);
		// move
		bool reach = false;
		while (!reach) {
			for (int j = 0; j < groups.size(); j++) {
				vector<int> peers = GetPeers(groups[j], robot, task, tID2index, stepNum - i - 1); // same group and to be docked group
				groups[j].PathPlanning(world, task->allTargets[stepNum - i - 1], peers);
				groups[j].TrialMove();
				if (!world->CollisionCheck(groups[j].GetRobotPos(), groups[j].GetRobotIds(),peers)) {
					groups[j].Move(world);
				}
			}
			// check, if leader robots reach targets, reach = true
			reach = CheckReach(groups);
			RecordRobotPosition(robot);
			Sleep(3000);
		}
		world->Display("all");
		string str1 = "Finished to move all robots to the targets of layer ";
		string str2 = to_string(stepNum - i - 1);
		RecordLog(str1 + str2);
		cout << str1 + str2 << endl;
		//system("pause");
		// dock
		groups = Dock(robot, task, tID2index, stepNum - i - 1);
	}
	//system("pause");
	Recover(task);
	return 0;
}


void ExtendTask(BinNode<vector<int>>* assNode, BinNode<char>* segNode, Task* task, MatrixMap* map, int depth, int obj) {
	// end condition
	if (!assNode) return;   // tree node empty
	if (assNode->data.size() <= 1) return;  // cannot be extended anymore

	if (depth == obj) {
		ExtendAction(assNode, segNode, task, map);
	}
	ExtendTask(assNode->lChild, segNode->lChild, task, map, depth + 1, obj);
	ExtendTask(assNode->rChild, segNode->rChild, task, map, depth + 1, obj);
}

void ExtendAction(BinNode<vector<int>>* assNode, BinNode<char>* segNode, Task* task, MatrixMap* map) {
	// find the position of target id in currentTargets
	vector<int> lcomponents = assNode->lChild->data;
	vector<int> left_ids;
	for (int i = 0; i < lcomponents.size(); i++)
		for (int j = 0; j < task->currentTargets.size(); j++)
			if (task->currentTargets[j]->id == lcomponents[i])    // ids[i] <-> components[i]
				left_ids.push_back(j);
	//
	vector<int> rcomponents = assNode->rChild->data;
	vector<int> right_ids;
	for (int i = 0; i < rcomponents.size(); i++)
		for (int j = 0; j < task->currentTargets.size(); j++)
			if (task->currentTargets[j]->id == rcomponents[i])    // ids[i] <-> components[i]
				right_ids.push_back(j);

	// �ƶ�һ������¼һ��, ̽�����=��ֹ����
	int step = 1; // ����
	int detect_dist = 2;  // ̽�����
	bool done = false;   // flag indicates the movement finish
	int total_move = 0;  // �������ƶ�����

	while (!done) {
		if (segNode->data == 'x') {   // ��x�������  // lcomponent��targets�㶼x+1, rcomponent��x-1
			{// left
				done = true;  // �Ƿ� û���ϰ�
				vector<TaskPoint*> tempTargets(task->currentTargets); // ��װ�ƶ�
				for (int i = 0; i < lcomponents.size(); i++) {   // component�ƶ�
					//cout << " xl " << i << endl;
					int temp_pos = tempTargets[left_ids[i]]->taskPoint.x + step;
					// collision, check map. ����������������ϰ���ķ�ӦӦ����ͬ
					if (map->TaskCheck(temp_pos, tempTargets[left_ids[i]]->taskPoint.y, assNode->data, detect_dist))  // ���û���ϰ�
						tempTargets[left_ids[i]]->taskPoint.x = temp_pos;
					else   // ����������ϰ�
						done = false;
				}

				if (done) {  // �������component��û�������ϰ�
					task->currentTargets.assign(tempTargets.begin(), tempTargets.end());
					total_move += step;
					// update map
					map->UpdateTaskmap(tempTargets);
				}
			}
			{// right
				done = true;  // �Ƿ� û���ϰ�
				vector<TaskPoint*> tempTargets(task->currentTargets);
				for (int i = 0; i < rcomponents.size(); i++) {
					//cout << " rl " << i << endl;
					int temp_pos = tempTargets[right_ids[i]]->taskPoint.x - step;
					//
					if (map->TaskCheck(temp_pos, tempTargets[right_ids[i]]->taskPoint.y, assNode->data, detect_dist))   // ���û���ϰ�
						tempTargets[right_ids[i]]->taskPoint.x = temp_pos;
					else   // ������ϰ�
						done = false;
				}
				if (done) {  // �������component��û�������ϰ�
					task->currentTargets.assign(tempTargets.begin(), tempTargets.end());
					total_move += step;
					// update map
					map->UpdateTaskmap(tempTargets);
				}
			}
		}
		else if (segNode->data == 'y') {  // ��y�������
			{// left
				done = true;  // �Ƿ� û���ϰ�
				vector<TaskPoint*> tempTargets(task->currentTargets); // ��װ�ƶ�
				for (int i = 0; i < lcomponents.size(); i++) {
					//cout << " yl " << i << endl;
					int temp_pos = tempTargets[left_ids[i]]->taskPoint.y + step;
					// collision, check map. ����������������ϰ���ķ�ӦӦ����ͬ
					if (map->TaskCheck(tempTargets[left_ids[i]]->taskPoint.x, temp_pos, assNode->data, detect_dist))   // ���û���ϰ�
						tempTargets[left_ids[i]]->taskPoint.y = temp_pos;
					else   // ������ϰ�
						done = false;
				}
				if (done) {  // �������component��û�������ϰ�
					task->currentTargets.assign(tempTargets.begin(), tempTargets.end());
					total_move += step;
					// update map
					map->UpdateTaskmap(tempTargets);
				}
			}
			{// right
				done = true;  // �Ƿ� û���ϰ�
				vector<TaskPoint*> tempTargets(task->currentTargets);
				for (int i = 0; i < rcomponents.size(); i++) {
					//cout << " yr " << i << endl;
					int temp_pos = tempTargets[right_ids[i]]->taskPoint.y - step;
					//
					if (map->TaskCheck(tempTargets[right_ids[i]]->taskPoint.x, temp_pos, assNode->data, detect_dist))   // ���û���ϰ�
						tempTargets[right_ids[i]]->taskPoint.y = temp_pos;
					else   // ������ϰ�
						done = false;
				}
				if (done) {  // �������component��û�������ϰ�
					task->currentTargets.assign(tempTargets.begin(), tempTargets.end());
					total_move += step;
					// update map
					map->UpdateTaskmap(tempTargets);
				}
			}
		}
		// else segNode containes nonsenses
		// end condition: if displacement >= Num, done = true
		if (total_move >= step * 2) break;
		else done = false;
	}
	/*
	if (done) {// assembly tree��һ�����,����alltargets
		vector<TaskPoint*> tempTargets;
		for (int i = 0; i < task->currentTargets.size(); i++) {
			TaskPoint* temp = new TaskPoint();
			temp->id = task->currentTargets[i]->id;
			temp->taskPoint.x = task->currentTargets[i]->taskPoint.x;
			temp->taskPoint.y = task->currentTargets[i]->taskPoint.y;
			tempTargets.push_back(temp);
		}
		task->allTargets.push_back(tempTargets);
	}
	// display
	cout << endl;
	map->Display();
	cout << endl;
	*/
}

// assign the task to the closest robots using optimization (or bid)
// from task->allTargets[j][i]->taskpoint.x(y)
// to robot[i]->initPosition.x(y)
void AssignTaskToRobot(Task* task, vector<Robot*> robot) {
	//
	float closestDistance;
	float distance;   // distance between robots and tasks
	bool assigned;
	vector<int> assignedTaskID;
	for (int i = 0; i < task->robotNum; i++) {
		closestDistance = 100000;   // max is initialized as infinity
		int roboX = robot[i]->initPosition.x;
		int roboY = robot[i]->initPosition.y;
		for (int j = 0; j < task->taskNum; j++) {
			// if task has been assigned, break
			assigned = false;
			for (int k = 0; k < assignedTaskID.size(); k++)
				if (assignedTaskID[k] == task->allTargets.back()[j]->id) {
					assigned = true;
					break;
				}
			if (assigned) continue;
			// calculate the distance
			int taskX = task->allTargets.back()[j]->taskPoint.x;
			int taskY = task->allTargets.back()[j]->taskPoint.y;
			distance = abs(roboX - taskX) + abs(roboY - taskY); // Manhattan distance
			//
			//cout << endl << "X: " << roboX << "    Y: " << roboY << "   distance: " << distance;
			if (distance < closestDistance) {
				closestDistance = distance;
				robot[i]->taskID = task->allTargets.back()[j]->id;
			}
		}
		assignedTaskID.push_back(robot[i]->taskID);
	}
	// see the assignment
	cout << endl;
	for (int i = 0; i < task->robotNum; i++) {
		cout << "Assign robot " << robot[i]->id << ": task ID " << robot[i]->taskID << endl;
	}
	cout << endl;
	RecordLog("Success to assign tasks to robots!");
}

void AssignCurrrentTargetToRobot(vector<Robot*> robot, vector<TaskPoint*> allTargets) {
	for (int j = 0; j < robot.size(); j++) {  // assign target points
		for (int k = 0; k < allTargets.size(); k++) {
			if (robot[j]->taskID == allTargets[k]->id) {
				robot[j]->targetPosition.x = allTargets[k]->taskPoint.x;
				robot[j]->targetPosition.y = allTargets[k]->taskPoint.y;
				break;
			}
		}
	}
}

// task ID->robot index // robot ID -> robot index
vector<vector<int>> IDtoIndex(vector<Robot*> robot) {
	int roboNum = robot[0]->robotNumber;  // task number = robot number
	vector<int> tID2index; // task ID->robot index
	vector<int> rID2index; // robot ID -> robot index
	vector<vector<int>> ID2index;  // ID2index[0] = tID2index  // ID2index[1] = rID2index; 
	for (int i = 0; i <= roboNum; i++) {
		{
			int taskID = i;   // task ID : 1~task number
			int index = 0;
			for (int j = 0; j < roboNum; j++)
				if (robot[j]->taskID == taskID) { index = j;  break; }
			tID2index.push_back(index);
			//cout << "task ID: " << taskID << "   index: " << index << endl;
		}
		{
			int robotID = i;   // robot ID : 1~robot number
			int index = 0;
			for (int j = 0; j < roboNum; j++)
				if (robot[j]->id == robotID) { index = j;  break; }
			rID2index.push_back(index);
			//cout << "robot ID: " << robotID << "   index: " << ind << endl;
		}
	}
	ID2index.push_back(tID2index);
	ID2index.push_back(rID2index);
	return ID2index;
}

// get the peers' IDs of robot group
vector<int> GetPeers(RobotGroup group, vector<Robot*> robot, Task* task, vector<int> tID2index, int layer) {
	vector<BinNode<vector<int>>*> nodeVec = task->AssemblyTree.getLayerNode(task->AssemblyTree.root(), 0, layer, nodeVec);
	vector<int> peerIDs;
	for (int i = 0; i < nodeVec.size(); i++) { // check each group
		bool isPeer = false;
		for (int j = 0; j < nodeVec[i]->data.size(); j++) { // each ID
			int robotID = robot[tID2index[nodeVec[i]->data[j]]]->id;
			peerIDs.push_back(robotID);
			if (group.robot[0]->id == robotID) isPeer = true; // pair the group
		}
		if (isPeer) return peerIDs;
		peerIDs.swap(vector<int>());
	}
	return peerIDs;
}

// record the current position of robots
bool RecordRobotPosition(vector<Robot*> robots) {
	ofstream f;
	f.open("../TestRobot/Robot_Current_Position.txt", ofstream::out);
	if (f) {
		f << robots.size() << endl;
		RecordLog("RecordCurrentAndTargetPosition:");
		RecordLog("RobotId   CurrentPosition   TargetPosition");
		for (int i = 0; i < robots.size(); ++i) {
			f << robots[i]->id << "," << robots[i]->currentPosition.x+1 << "," << robots[i]->currentPosition.y+1 << ","
				<< robots[i]->targetPosition.x+1 << "," << robots[i]->targetPosition.y+1 << endl;
			//RecordLog("   " + to_string(i) + "          [" + to_string(robotCurrentPosition[i - 1].x + 1) + ","
			//	+ to_string(robotCurrentPosition[i - 1].y + 1) + "]              [" + to_string(robotTargetPosition[i - 1].x + 1) +
			//	"," + to_string(robotTargetPosition[i - 1].y + 1) + "]");
		}
	}
	else
		return false;
	f.close();
	return true;
}

// recover the task and robot data
void Recover(Task* task) {
	ofstream f;
	f.open("../TestRobot/Robot_Current_Position.txt", ofstream::out);
	if (f) {
		f << task->robotNum << endl;
		for (int i = 0; i < task->robotNum; ++i) {
			f << task->startPoints[i]->id << "," << task->startPoints[i]->taskPoint.x + 1 << "," 
				<< task->startPoints[i]->taskPoint.y + 1 << "," << task->finalTargets[i]->taskPoint.x + 1
				<< "," << task->finalTargets[i]->taskPoint.y + 1 << endl;
		}
	}
	f.close();
	/*
	f.open("../TestRobot/Task.txt", ofstream::out);
	if (f) {
		f << task->taskNum << endl;
		for (int i = 0; i < task->taskNum; i++) {
			//
			f << task->finalTargets[i]->id << " " << task->finalTargets[i]->taskPoint.x << " "
				<< task->finalTargets[i]->taskPoint.y << endl;
			// Log
		}
	}
	f.close();
	*/
}

// check all the robots whether reach their targets
bool CheckReach(vector<RobotGroup> groups) {
	for (int i = 0; i < groups.size(); i++) {
		if (groups[i].robot[0]->currentPosition.x != groups[i].robot[0]->targetPosition.x
			|| groups[i].robot[0]->currentPosition.y != groups[i].robot[0]->targetPosition.y) {
			return false;
		}
	}
	return true;
}

// dock // put robots into one group
vector<RobotGroup> Dock(vector<Robot*> robot, Task* task, vector<int> tID2index, int layer) {  // layer = stepNum - i - 1
	// get nodes of one layer, task->AssemblyTree  // all nodes of one layer of task tree
	vector<BinNode<vector<int>>*> nodeVec = task->AssemblyTree.getLayerNode(task->AssemblyTree.root(), 0, layer, nodeVec);
	// see the nodes
	cout << "all groups in nodes: ";
	for (int j = 0; j < nodeVec.size(); j++) {
		cout << "(";
		for (int k = 0; k < nodeVec[j]->data.size(); k++) {
			cout << nodeVec[j]->data[k] << ",";
		}
		cout << "),";
	}
	cout << endl;

	// push robots into one group
	vector<RobotGroup> groups;
	for (int j = 0; j < nodeVec.size(); j++) {  // for each one node 
		vector<Robot*> tempGroup;
		for (int k = 0; k < nodeVec[j]->data.size(); k++) {  // for each one robot
			tempGroup.push_back(robot[tID2index[nodeVec[j]->data[k]]]); // int taskID = nodeVec[j]->data[k];
		}
		RobotGroup robotGroup(tempGroup);  // robot group
		groups.push_back(robotGroup);
	}
	// see the groups // assign the leaders 
	for (int j = 0; j < groups.size(); j++) {
		//cout << "group " << j << ":";  groups[j].Display();  cout << endl;
		groups[j].AssignLeaders();
	}
	RecordLog("Finished to dock!");
	cout << endl << "Finished to dock!" << endl;
	return groups;
}