#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <string>
#include <thread>
#include <math.h>
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <mutex>
#include <windows.h>
using namespace std;
bool RecordLog(string s);
//��ͼ����Ķ���
class Point{         // the point in the map
public:
	Point() :x(0), y(0), Leaderflag(false), Dockedflag(false) {} 
	Point(int tx, int ty){                      // target points
		x = tx;
		y = ty;
	}
	bool operator==(const Point& r) const {     // reload logic operation ==
		return (x == r.x) && (y == r.y);
	}
	bool operator < (const Point &r) const {    // reload logic operation <
		int a = x * x + y * y;
		int b = r.x * r.x + r.y * r.y;
		if (a < b){
			return true;
		}
		else if (a == b){                      // if a and b have the same distance to origin, less with less x
			if (x < r.x)
				return true;
			else
				return false;
		}
		else{
			return false;
		}
	}
	Point up(){
		return Point(x - 1 , y);
	}
	Point down(){
		return Point(x + 1 , y);
	}
	Point left(){
		return Point(x , y - 1);
	}
	Point right(){
		return Point(x , y + 1);
	}
	friend class Rule;
	friend class Robot;
	friend class Task;
	friend int main();
	int x;
	int y;
	bool Dockedflag;
	bool Leaderflag;
	int LeaderNum = -1;
private:
};
//Ȩ�ص���Ķ���
class WeightPoint{           // ÿ���������������ֵ��1�����ߣ�-1�ǲ�����
private:
	int up;
	int down;
	int left;
	int right;
public:
	WeightPoint() :up(0), down(0), left(0), right(0){}
	bool operator == (const WeightPoint& r) const {
		return (up == r.up) && (down == r.down) && (left == r.left) && (right == r.right);
	}
	friend class Rule;
	friend class Robot;
	friend int main();
};

//������Ķ���
class Task{            // class task to describe the details of the task
public:
	Task() :id(0), publishTime(0), waitTime(0), source(), target() {}
	bool operator == (const Task& r) const {
		return (id == r.id) &&
			(publishTime == r.publishTime) &&
			(waitTime == r.waitTime) &&
			(source == r.source) &&
			(target == r.target);
	}
	friend class Rule;
	friend class Robot;
	friend int main();
	int id;            // task id
	int publishTime;   // the time generating task
	int waitTime;      // the time next task wait
	Point source;      // starting point of task
	Point target;      // end point of task
private:
};
struct CutResult           // �滮�����˶Խ�ʱ����Ŀ����ֵĲ���
{
	vector<Task> Part1, Part2;
};
vector<Point> robotCurrentPositionCopy;
vector<vector<int> > AvoidMap;              // obstacle map?? 2 dimensional vector
int Column, Row;
//������Ķ���
class Rule{
private:
	int mapRowNumber;                      // the number of map row
	int mapColumnNumber;                   // the number of map column
	int robotNumber;                       // the number of robot
	int globalTime;                        // the time of system
	int taskNum;                           // the number of task
	int taskId;                            // taskId is used to record the id of task
	vector<float> bidTable;                // the talbe of all robot's bid price
	
	vector<Point> robotInitPosition;       // the init position of robot
	vector<Point> robotCurrentPosition;    // the current position of robot
	vector<Point> robotTargetPosition;     // the target position of robot
	vector<Task> ToDoTask, DoingTask, DoneTask; // three vector of task
	vector<Task> waitToAssigned;             // wait to be assigned
	vector<vector<WeightPoint>> weightMap;   // the weight of map
public:
	vector<vector<int> > map;               // use two dimension to record the map
	//changed by jiaming, it supposed to be private

	Rule() :mapRowNumber(0),mapColumnNumber(0),globalTime(0),robotNumber(0),taskNum(0),
		taskId(0){cout << "Construct a Object of Class Rule " << endl; RecordLog("Construct a Object of Class Rule ");}
	inline int getMapRowNumber(){return mapRowNumber;}
	inline int getMapColumnNumber(){return mapColumnNumber;}
	inline int getRobotNumber(){return robotNumber;}
	inline bool setGlobalTime(int newGlobalTime){globalTime = newGlobalTime;return true;}
	inline int getGlobalTime(){return globalTime;}
	vector<vector<int> > getMap(){return AvoidMap;}
	vector<vector<WeightPoint> > getWeightMap(){ return weightMap; }
	Point getRobotInitPosition(int RobotNumber){return robotInitPosition[RobotNumber];}
	bool setRobotCurrentPosition(int RobotNumber, Point Position){
		robotCurrentPosition[RobotNumber] = Position;
		robotCurrentPositionCopy[RobotNumber] = Position;
		return true;
	}
	bool setRobotTargetPosition(int RobotNumber, Point Position){
		robotTargetPosition[RobotNumber] = Position;
		return true;
	}
	bool Dock();
	bool ReadMap();
	bool GlobalTime();
	bool ReadRobotInitPosition();
	bool RecordCurrentAndTargetPosition();
	bool GenerateTask();
	bool ReadTask();
	
	CutResult Cut(vector<Task> InputVector);
	CutResult MoveRow(CutResult InputVector);
	CutResult MoveVector(CutResult InputVector);
	int PrintVector(vector<Task> InputVector);
	int ShortDistance(Point& source, Point& target);
	bool AssignTask();
	friend class Robot;
	friend int main();
};

//�������ඨ��
class Robot {
private:
	static int robotNumber;                 //����������
	static map<Point, int> robotPosition;   //������λ��
	static vector<float> bidTable;   // �����˾��۱�
	static mutex mu;                 // ��ͼ��
	int Id;                   // robot Id , start from zero
	int Status;               // robot status,1 means to go to source of task , 2 means to go to end of task
	bool isBack;              // robot is back to init position
	int waitTime;             // robot waitTime
	float bidPrice;           // the price robot bid for task
	int mapRowNumber;         // the row number of map
	int mapColumnNumber;      // the column number of map
	int distance;             // �������ƶ�����
	int fightTime;            // �����˵�ǰ����ƶ�����
	int alreadyWait;          // �����˳�ͻ�ȴ�ʱ��
	Point initPosition;       // initPosition of robot
	Point nextPosition;       // ������֮ǰ��λ��
	Point currentPosition;    // currentPosition of robot
	Point targetPosition;     // targetPosition of robot
	vector<Task> doingTask;   // the task robot is doing
	vector<Task> todoTask;    // the queue of TodoTask
	vector<Task> doneTask;    // the queue of DoneTask
	vector<Task> updateTask;  // ��������õ���������
	vector<Point> planPath;   // the path of robot
	vector<vector<int>> workmap;  // the map robot walks
	vector<vector<WeightPoint>> weightMap;    // the map robot think the map
public:
	Robot() :Id(0), Status(0), isBack(false), waitTime(-1), bidPrice(0),
		mapRowNumber(0), mapColumnNumber(0), distance(0), fightTime(0), alreadyWait(0),
		initPosition(), nextPosition(), currentPosition(), targetPosition() {
		++robotNumber;
		cout << "Construct Robot Number " << robotNumber << endl;
		RecordLog("Construct Robot Number " + to_string(robotNumber));
	}
	bool operator == (const Robot& r) const {
		return (Id == r.Id) && (Status == r.Status) && (isBack == r.isBack) && (waitTime && r.waitTime) &&
			(bidPrice == r.bidPrice) && (mapRowNumber == r.mapRowNumber) &&
			(distance == r.distance) && (fightTime == r.fightTime) &&
			(mapColumnNumber == r.mapColumnNumber) && (initPosition == r.initPosition) &&
			(nextPosition == r.nextPosition) && (alreadyWait == r.alreadyWait) &&
			(currentPosition == r.currentPosition) && (targetPosition == r.targetPosition) &&
			(doingTask == r.doingTask) && (planPath == r.planPath) && (todoTask == r.todoTask) &&
			(doneTask == r.doneTask) && (workmap == r.workmap) && (weightMap == r.weightMap);
	}
	void Bid(Task& task);      // robot bid for task
	bool TaskOrder();    // reorder the tasks of robot
	vector<Point> ShortestPath(Point& source, Point& target);   // plan the route of robot
	vector<Point> ShortestPath(Point& source, Point& target, Point Obs);
	int ShortestDistance(Point& source, Point& target);         // ��㵽�յ�ľ���
	bool Move();         // robot move
	friend int main();
	void Avoid(Point InputPoint);
	void AvoidClear(Point InputPoint);
	Point Robot::Dock(Point InputNextPosition);
	friend class Rule;
};

//Jiaming's Code
vector<Task> TaskToDo, SingleSet, CombineSet;
vector<int> PublishNum;
vector<vector<int>> DockMap;
int TaskNum, SingleRobotNum = 0;
bool AllFinishFlag = false;
int Rule::PrintVector(vector<Task> InputVector)
{
	for (int i = 0; i < InputVector.size(); ++i) {
		cout << InputVector[i].id << " " << InputVector[i].source.x << " " << InputVector[i].source.y << endl;
	}
	return 0;
}
// ��ͼ��ı�ǣ� 0��free,1���ϰ���2�ǻ�����
Point Robot::Dock(Point InputNextPosition)
{
	Point OutNextPosition;
	OutNextPosition.x = 0;
	OutNextPosition.y = 0;
	int OffsetX = 0, OffsetY = 0;    // ����Խ�����������������leader�����ƫ��
	if (currentPosition.x > 0 && currentPosition.y > 0) {  // ǰ�����ҽ�����һ����һ�������ˣ���dock
		if (DockMap[currentPosition.x - 1][currentPosition.y] == 2 || DockMap[currentPosition.x + 1][currentPosition.y] == 2 ||
			DockMap[currentPosition.x][currentPosition.y - 1] == 2 || DockMap[currentPosition.x][currentPosition.y + 1] == 2) {
			currentPosition.Dockedflag = true;
		}
	}
	if (currentPosition.x > 0 && currentPosition.y > 0) {    
		if (DockMap[currentPosition.x][currentPosition.y - 1] != 2 &&
			DockMap[currentPosition.x + 1][currentPosition.y] != 2) {  // ��� y-1 �� x+1������û�л�����, �Ǿ���leader
			currentPosition.Leaderflag = true; 
			OutNextPosition.x = OffsetX;
			OutNextPosition.y = OffsetY;
			return OutNextPosition;
		}
		else {    // �Խ��������л����˵�ƫ��
			for (int j = 1; j < TaskNum; ++j) {
				if (DockMap[currentPosition.x][currentPosition.y - j] != 2) {
					OffsetY = j;
					break;
				}
			}
			for (int j = 1; j < TaskNum; ++j) {
				if (DockMap[currentPosition.x + j][currentPosition.y] != 2) {
					OffsetX = j;
					break;
				}
			}
			/*for (int j = 0; j < TaskNum; ++j) {
				if (.x == currentPosition.x + OffsetX &&
					currentPosition.y == currentPosition.y - OffsetY) {
					currentPosition.LeaderNum = j;
					break;
				}
			}*/
			OutNextPosition.x = OffsetX;
			OutNextPosition.y = OffsetY;
			return OutNextPosition;
		}
	}
	/*for (int i = 0; i < robotCurrentPosition.size(); ++i) {
		if (robotCurrentPosition[i].x > 0 && robotCurrentPosition[i].y > 0) {
			if (DockMap[robotCurrentPosition[i].x][robotCurrentPosition[i].y - 1] != 2 &&
				DockMap[robotCurrentPosition[i].x + 1][robotCurrentPosition[i].y] != 2) {
				robotCurrentPosition[i].Leaderflag = true;
				robotCurrentPosition[i].LeaderNum = i;
			}
			else {
				for (int j = 1; j < robotCurrentPosition.size(); ++j) {
					if (DockMap[robotCurrentPosition[i].x][robotCurrentPosition[i].y - j] != 2) {
						OffsetY = j;
						break;
					}
				}
				for (int j = 1; j < robotCurrentPosition.size(); ++j) {
					if (DockMap[robotCurrentPosition[i].x + j][robotCurrentPosition[i].y] != 2) {
						OffsetX = j;
						break;
					}
				}
			}
			for (int j = 0; j < robotCurrentPosition.size(); ++j) {
				if (robotCurrentPosition[j].x == robotCurrentPosition[i].x + OffsetX &&
					robotCurrentPosition[j].y == robotCurrentPosition[i].y - OffsetY) {
					robotCurrentPosition[i].LeaderNum = j;
					break;
				}
			}
		}
		//if (robotCurrentPosition[i].Dockedflag = true && robotCurrentPosition[i].y - 1 != 2 && robotCurrentPosition[i].x + 1 != 2) {
	}*/
}
void Robot::Avoid(Point InputPoint)    // ��������˵�λ�ã�������������Ŀ��н�
{
	bool ShouldAvoidFlag = true;
	for (int i = 0; i < TaskToDo.size(); ++i) {
		if (TaskToDo[i].source.x == InputPoint.x && TaskToDo[i].source.y == InputPoint.y) {
			ShouldAvoidFlag = false;
			break;
		}
	}
	if (ShouldAvoidFlag == true) {
		if (InputPoint.x > 1) {
			if (AvoidMap[InputPoint.x - 2][InputPoint.y] == 0) {
				weightMap[InputPoint.x - 1][InputPoint.y].up = 1;
			}
			else {
				weightMap[InputPoint.x - 1][InputPoint.y].up = -1;
			}
		}
		if (InputPoint.x < Row - 2) {
			if (AvoidMap[InputPoint.x + 2][InputPoint.y] == 0) {
				weightMap[InputPoint.x + 1][InputPoint.y].down = 1;
			}
			else {
				weightMap[InputPoint.x + 1][InputPoint.y].down = -1;
			}
		}
		if (InputPoint.y > 1) {
			if (AvoidMap[InputPoint.x][InputPoint.y - 2] == 0) {
				weightMap[InputPoint.x][InputPoint.y - 1].left = 1;
			}
			else {
				weightMap[InputPoint.x][InputPoint.y - 1].left = -1;
			}
		}
		if (InputPoint.y < Row - 2) {
			if (AvoidMap[InputPoint.x][InputPoint.y + 2] == 0) {
				weightMap[InputPoint.x][InputPoint.y + 1].right = 1;
			}
			else {
				weightMap[InputPoint.x][InputPoint.y + 1].right = -1;
			}
		}
	}
}
void Robot::AvoidClear(Point InputPoint) // �жϻ�������һ���Ƿ��ͻ���������ͻ
{
	if (InputPoint.x > 1) {
		if (AvoidMap[InputPoint.x - 2][InputPoint.y] != 1 && weightMap[InputPoint.x - 1][InputPoint.y].up == -1) {
			weightMap[InputPoint.x - 1][InputPoint.y].up = 1;
		}
	}
	if (InputPoint.x < Row - 2) {
		if (AvoidMap[InputPoint.x + 2][InputPoint.y] != 1 && weightMap[InputPoint.x + 1][InputPoint.y].down == -1) {
			weightMap[InputPoint.x + 1][InputPoint.y].down = 1;
		}
	}
	if (InputPoint.y > 1) {
		if (AvoidMap[InputPoint.x][InputPoint.y - 2] != 1 && weightMap[InputPoint.x][InputPoint.y - 1].left == -1) {
			weightMap[InputPoint.x][InputPoint.y - 1].left = 1;
		}
	}
	if (InputPoint.y < Row - 2) {
		if (AvoidMap[InputPoint.x][InputPoint.y + 2] != 1 && weightMap[InputPoint.x][InputPoint.y + 1].right == -1) {
			weightMap[InputPoint.x][InputPoint.y + 1].right = 1;
		}
	}
}
CutResult Rule::MoveRow(CutResult InputVector)  // �������鰴y�����м�������ɢ
{
	int Direction1 = 0, FactorFor1 = 1, FactorFor2 = -1;//0 means up, 1 means down
	Task TmpResult;
	if (InputVector.Part1.size() != 1) { // part1 ���ܼ�������
		for (int i = 0; i < InputVector.Part1.size(); ++i) {
			InputVector.Part1[i].source.x = InputVector.Part1[i].source.x + FactorFor1;
		}
		Cut(InputVector.Part1);
	}
	else {
		++SingleRobotNum;
		TmpResult.id = InputVector.Part1[0].id;
		TmpResult.source.x = InputVector.Part1[0].source.x;
		TmpResult.source.y = InputVector.Part1[0].source.y;
		SingleSet.push_back(TmpResult);
	}
	if (InputVector.Part2.size() != 1) { // part 2 ���ܼ�������
		for (int i = 0; i < InputVector.Part2.size(); ++i) {
			InputVector.Part2[i].source.x = InputVector.Part2[i].source.x + FactorFor2;
		}
		Cut(InputVector.Part2);
	}
	else {
		++SingleRobotNum;
		TmpResult.id = InputVector.Part2[0].id;
		TmpResult.source.x = InputVector.Part2[0].source.x;
		TmpResult.source.y = InputVector.Part2[0].source.y;
		SingleSet.push_back(TmpResult);
	}
	/*if (SingleRobotNum == TaskNum) {
		FinishFlag = true;
	}*/
	return InputVector;
}
CutResult Rule::MoveVector(CutResult InputVector) // ����������x���������ɢ
{
	int Direction1 = rand() % 3, Direction2, FactorFor1, FactorFor2;
	Task TmpResult;
	if (Direction1 == 0) {//0: up
		FactorFor1 = 1;
		Direction2 = rand() % 2 + 4;
		if (Direction2 == 4) {//4: right
			FactorFor2 = 1;
		}
		else {//5: down
			FactorFor2 = -1;
		}
	}
	else if (Direction1 == 1) {//1: down
		FactorFor1 = -1;
		Direction2 = rand() % 2 + 6;
		FactorFor2 = 1;//6: right 7: up
	}
	else {//2: left
		FactorFor1 = -1;
		Direction2 = rand() % 2 + 8;
		if (Direction2 == 8) {//8: down
			FactorFor2 = -1;
		}
		else {//9: up 10:right
			FactorFor2 = 1;
		}
	}
	if (InputVector.Part1.size() != 1) {//if size is not 1 then move
		if (Direction1 == 0 || Direction1 == 1) {
			for (int i = 0; i < InputVector.Part1.size(); ++i) {
				InputVector.Part1[i].source.x = InputVector.Part1[i].source.x + FactorFor1;
			}
		}
		else {
			for (int i = 0; i < InputVector.Part1.size(); ++i) {
				InputVector.Part1[i].source.y = InputVector.Part1[i].source.y + FactorFor1;
			}
		}
		Cut(InputVector.Part1);
	}
	else {
		++SingleRobotNum;
		TmpResult.id = InputVector.Part1[0].id;
		TmpResult.source.x = InputVector.Part1[0].source.x;
		TmpResult.source.y = InputVector.Part1[0].source.y;
		SingleSet.push_back(TmpResult);
	}//otherwise don't move and add a single robot
	if (InputVector.Part2.size() != 1) {
		if (Direction2 == 5 || Direction2 == 7 || Direction2 == 8 || Direction2 == 9) {
			for (int i = 0; i < InputVector.Part2.size(); ++i) {
				InputVector.Part2[i].source.x = InputVector.Part2[i].source.x + FactorFor2;
			}
		}
		else {
			for (int i = 0; i < InputVector.Part2.size(); ++i) {
				InputVector.Part2[i].source.y = InputVector.Part2[i].source.y + FactorFor2;
			}
		}
		Cut(InputVector.Part2);
	}
	else {
		++SingleRobotNum;
		TmpResult.id = InputVector.Part2[0].id;
		TmpResult.source.x = InputVector.Part2[0].source.x;
		TmpResult.source.y = InputVector.Part2[0].source.y;
		SingleSet.push_back(TmpResult);
	}
	return InputVector;
}
CutResult Rule::Cut(vector<Task> InputVector) // �ָ��������
{
	bool MultiRowFlag = false;
	int CutPosition;
	for (int i = 1; i < InputVector.size(); ++i) { // �жϷָ�λ�����Ķ��� i-1.source.x != i.source.x
		if (InputVector[i - 1].source.x != InputVector[i].source.x) {
			MultiRowFlag = true;
			CutPosition = i;
			break;
		}
	}
	CutResult Result;
	Task TmpResult;
	if (MultiRowFlag) {
		PublishNum.push_back(InputVector.size());
		for (int i = 0; i < CutPosition; ++i) {
			TmpResult.id = InputVector[i].id;
			TmpResult.source.x = InputVector[i].source.x;
			TmpResult.source.y = InputVector[i].source.y;
			Result.Part1.push_back(TmpResult);
			CombineSet.push_back(TmpResult);
		}
		for (int i = CutPosition; i < InputVector.size(); ++i) {
			TmpResult.id = InputVector[i].id;
			TmpResult.source.x = InputVector[i].source.x;
			TmpResult.source.y = InputVector[i].source.y;
			Result.Part2.push_back(TmpResult);
			CombineSet.push_back(TmpResult);
		}
	}
	else if (InputVector.size() == 2) {
		TmpResult.id = InputVector[0].id;
		TmpResult.source.x = InputVector[0].source.x;
		TmpResult.source.y = InputVector[0].source.y;
		Result.Part1.push_back(TmpResult);
		SingleSet.push_back(TmpResult);
		TmpResult.id = InputVector[1].id;
		TmpResult.source.x = InputVector[1].source.x;
		TmpResult.source.y = InputVector[1].source.y;
		Result.Part2.push_back(TmpResult);
		SingleSet.push_back(TmpResult);
	}
	else if (InputVector.size() % 2 == 0) {
		PublishNum.push_back(InputVector.size());
		CutPosition = InputVector.size() / 2;
		for (int i = 0; i < CutPosition; ++i) {
			TmpResult.id = InputVector[i].id;
			TmpResult.source.x = InputVector[i].source.x;
			TmpResult.source.y = InputVector[i].source.y;
			Result.Part1.push_back(TmpResult);
			CombineSet.push_back(TmpResult);
		}
		for (int i = CutPosition; i < InputVector.size(); ++i) {
			TmpResult.id = InputVector[i].id;
			TmpResult.source.x = InputVector[i].source.x;
			TmpResult.source.y = InputVector[i].source.y;
			Result.Part2.push_back(TmpResult);
			CombineSet.push_back(TmpResult);
		}
	}
	else if (InputVector.size() == 1) {
		TmpResult.id = InputVector[0].id;
		TmpResult.source.x = InputVector[0].source.x;
		TmpResult.source.y = InputVector[0].source.y;
		Result.Part1.push_back(TmpResult);
		SingleSet.push_back(TmpResult);
	}
	else {
		PublishNum.push_back(InputVector.size());
		CutPosition = 1;
		//for (int i = 0; i < CutPosition; ++i) {
		TmpResult.id = InputVector[0].id;
		TmpResult.source.x = InputVector[0].source.x;
		TmpResult.source.y = InputVector[0].source.y;
		Result.Part1.push_back(TmpResult);
		CombineSet.push_back(TmpResult);
		//}
		for (int i = CutPosition; i < InputVector.size(); ++i) {
			TmpResult.id = InputVector[i].id;
			TmpResult.source.x = InputVector[i].source.x;
			TmpResult.source.y = InputVector[i].source.y;
			Result.Part2.push_back(TmpResult);
			CombineSet.push_back(TmpResult);
		}
	}
	if (MultiRowFlag) {
		Result = MoveRow(Result);
		MultiRowFlag = false;
	}
	else if (Result.Part1.size() == 1 && Result.Part2.size() == 1) { ; }
	else {
		Result = MoveVector(Result);
	}
	return Result;
}

bool 
Rule::ReadMap(){    // Read the map from given file and form the weight map
	ifstream f;
	f.open("../TestRobot/InitMap.txt", ifstream::in);
	if (f){
		f >> mapRowNumber;
		Row = mapRowNumber;
		char c;
		f >> c;
		f >> mapColumnNumber;
		Column = mapColumnNumber;
		for (int r = 0; r < mapRowNumber; ++r){
			vector<int> oneRow;
			for (int c = 0; c < mapColumnNumber; ++c){
				int onepoint;
				char t;
				f >> onepoint;
				oneRow.push_back(onepoint);
				if (c != (mapColumnNumber - 1))
					f >> t;//t may be the next-line signal
			}
			//map.push_back(oneRow);
			AvoidMap.push_back(oneRow);
		}
	}
	else{
		cout << " Failed to open the InitMap.txt! " << endl;
		RecordLog("Failed to open the InitMap.txt!");
		return false;
	}
	DockMap = AvoidMap;
	f.close();
	//mapRowNumber = mapRowNumber - 7;
	for (int i = 0; i < mapRowNumber; ++i){
		vector<WeightPoint> oneRow;
		for (int j = 0; j < mapColumnNumber; ++j){
			WeightPoint t;
			if (AvoidMap[i][j] == 1){
				t.up = -1;
				t.down = -1;
				t.left = -1;
				t.right = -1;
			}
			else{
				if ((i - 1) >= 0 && (i - 1) < mapRowNumber){
					if (AvoidMap[i - 1][j] == 0)
						t.up = 1;
					else
						t.up = -1;
				}
				else
					t.up = -1;
				if ((i + 1) >= 0 && (i + 1) < mapRowNumber){
					if (AvoidMap[i + 1][j] == 0)
						t.down = 1;
					else
						t.down = -1;
				}
				else
					t.down = -1;
				if ((j - 1) >= 0 && (j - 1) < mapColumnNumber){
					if (AvoidMap[i][j - 1] == 0)
						t.left = 1;
					else
						t.left = -1;
				}
				else
					t.left = -1;
				if ((j + 1) >= 0 && (j + 1) < mapColumnNumber){
					if (AvoidMap[i][j + 1] == 0)
						t.right = 1;
					else
						t.right = -1;
				}
				else
					t.right = -1;
			}
			oneRow.push_back(t);
		}
		weightMap.push_back(oneRow);
	}
	cout << "Success to read the map from file InitMap.txt" << endl;
	RecordLog("Success to read the map from file InitMap.txt");
	return true;
}
bool 
Rule::GlobalTime(){   // record the globaltime and print the global time
	++globalTime;
	cout << "GlobalTime is " << globalTime << endl;
	RecordLog("GlobalTime is "+to_string(globalTime));
	return true;
}

bool 
Rule::RecordCurrentAndTargetPosition(){
	ofstream f;
	f.open("../TestRobot/Robot_Current_Position.txt", ofstream::out);
	if (f){
		f << robotCurrentPosition.size() << endl;
		RecordLog("RecordCurrentAndTargetPosition:");
		RecordLog("RobotId   CurrentPosition   TargetPosition");
		for (int i = 1; i <= robotCurrentPosition.size(); ++i){
			f << i << "," << robotCurrentPosition[i - 1].x + 1 << "," << robotCurrentPosition[i - 1].y + 1 << ","
				<< robotTargetPosition[i - 1].x + 1 << "," << robotTargetPosition[i - 1].y + 1 << endl;
			RecordLog("   " + to_string(i) + "          [" + to_string(robotCurrentPosition[i - 1].x + 1) + ","
				+ to_string(robotCurrentPosition[i - 1].y + 1) + "]              [" + to_string(robotTargetPosition[i - 1].x + 1) +
				"," + to_string(robotTargetPosition[i - 1].y + 1)+"]");
		}
	}
	else
		return false;
	f.close();
	return true;
}

bool 
Rule::ReadRobotInitPosition(){
	ifstream f;
	f.open("../TestRobot/Robot_Init_Position.txt", ifstream::in);
	if (f){
		f >> robotNumber;
		bidTable.resize(robotNumber);
		Point target = Point(-1,-1);
		for (int i = 0; i < robotNumber; ++i){
			int x, y;
			int tempdata;
			char tempchar;
			f >> tempdata;
			f >> tempchar;
			f >> tempdata;
			x = tempdata-1;
			f >> tempchar;
			f >> tempdata;
			y = tempdata - 1;
			Point t;
			t.x = x;
			t.y = y;
			robotInitPosition.push_back(t);
			robotTargetPosition.push_back(target);
		}
		robotCurrentPosition = robotInitPosition;  // the init of robotCurrentPosition is robotInitPosition
		robotCurrentPositionCopy = robotInitPosition;
		cout << "Success to read the robotInitPosition from file Robot_Init_Position.txt" << endl;
		RecordLog("Success to read the robotInitPosition from file Robot_Init_Position.txt");
	}
	else{
		return false;
	}
	f.close();
	return true;
}

bool
Rule::GenerateTask(){
	// ����������
	cout << "Please input the taskNum:" << endl;
	cin >> taskNum;
	/*ofstream f;
	int ptime = 0;
	f.open("../TestRobot/Task.txt", ofstream::out);
	if (f){
		f << taskNum << endl;
		for (int i = 1; i <= taskNum; ++i){
			Task tempTask;
			tempTask.id = taskId + 1;
			++ taskId;
			tempTask.publishTime = ptime;
			tempTask.waitTime = rand() % 3 + 2;
			do{
				tempTask.source.x = rand() % (mapRowNumber - 1) + 1;
				tempTask.source.y = rand() % mapColumnNumber;
				tempTask.target.x = rand() % (mapRowNumber - 1) + 1;
				tempTask.target.y = rand() % mapColumnNumber;
			} while (tempTask.source == tempTask.target ||
				map[tempTask.source.x][tempTask.source.y] == 1 ||
				map[tempTask.target.x][tempTask.target.y] == 1);
			ptime += tempTask.waitTime;
			f << tempTask.id << " " << tempTask.publishTime << " " <<
				tempTask.waitTime << " " << tempTask.source.x << " " <<
				tempTask.source.y << " " << tempTask.target.x << " " <<
				tempTask.target.y << endl;
		}
	}
	else
		cout << "Failed to open the target file . Please check the filename . " << endl;
	f.close();*/
	cout << "Success to generate task" << endl;
	return true;
}

bool 
Rule::ReadTask(){
	ifstream f;
	f.open("../TestRobot/Task.txt", ifstream::in);
	if (f){
		f >> taskNum;
		TaskNum = taskNum;
		Task tempTask;
		for (int i = 0; i < taskNum; ++i){
			f >> tempTask.id >> tempTask.source.x >> tempTask.source.y;
			TaskToDo.push_back(tempTask);
		}
		//TaskToDo = ToDoTask;
	}
	else
		cout << "Failed to open the target file . Please check the filename ." << endl;
	f.close();
	cout << "Finish to read task" << endl;
	RecordLog("Finish to read task");
	return true;
}

void 
Robot::Bid(Task& task){     // robot bid a price for the task
	float price = 0;
	Point finalPosition;
	int choice = 0;
	updateTask = doingTask;
	updateTask.push_back(task);
	if (doingTask.size() == 0){
		finalPosition = currentPosition;
		price = ShortestDistance(finalPosition, task.source) + doingTask.size() * doingTask.size();
	}
	else{
		finalPosition = doingTask.back().target;
		//̰���㷨
		if (choice == 0){
			price = ShortestDistance(finalPosition, task.source);
		}
	}
	bidTable[Id] = price;
}

bool 
Robot::TaskOrder(){          // reorder the todoTask,��ֻ����������һ���ӿڣ������Ҫ�������ڴ˽�����չ
	return true;
}

int
Robot::ShortestDistance(Point &source, Point &target){
	vector<Point> path;

	if (workmap[source.x][source.y] == 1 || workmap[target.x][target.y] == 1){
		cout << "�����յ����ϰ���" << endl;
	}

	if (source == target){
		path.push_back(source);
		return path.size();
	}
	// A* algorithm

	struct APoint{
		Point p;
		double g;
		double f;
		bool operator < (const APoint &a) const {
			return f < a.f;  // ��Сֵ����
		}
	};

	//source = Point(0, 13);
	//target = Point(17, 0);
	vector<Point> closeList;
	vector<APoint> openList;
	map<Point, Point> relation;
	APoint ap;
	ap.p = source;
	ap.g = 0;//gain
	ap.f = 0;//feed
	openList.push_back(ap);
	double g = 0;
	double h = 0;
	double f = 0;
	double t = 0;
	bool flag = true;
	while (openList.empty() == false && flag == true){
		ap = openList[0];
		openList.erase(openList.begin());
		Point cur = ap.p;
		closeList.push_back(cur);
		Point next;
		double gn = 0;

		for (int direct = 1; direct <= 4; ++direct){
			if (direct == 1){
				next = cur.up();
				gn = weightMap[ap.p.x][ap.p.y].up;
			}
			else if (direct == 2){
				next = cur.right();
				gn = weightMap[ap.p.x][ap.p.y].right;
			}
			else if (direct == 3){
				next = cur.down();
				gn = weightMap[ap.p.x][ap.p.y].down;
			}
			else if (direct == 4){
				next = cur.left();
				gn = weightMap[ap.p.x][ap.p.y].left;
			}
			if (next.x >= 0 && next.x < mapRowNumber && next.y >= 0 &&
				next.y < mapColumnNumber && workmap[next.x][next.y] == 0){
				//�����µ�F(N)

				int ci = -1;
				for (int i = 0; i < closeList.size(); ++i){
					if (closeList[i] == next){
						ci = i;
						break;
					}
				}

				if (ci == -1){
					int oi = -1;
					for (int i = 0; i < openList.size(); ++i){
						if (openList[i].p == next){
							oi = i;
							break;
						}
					}
					if (oi == -1){    //openList�в����ڱ����ڵ�
						g = ap.g + 1 + gn;
						h = abs(target.x - next.x) + abs(target.y - next.y);
						t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
							(target.y - next.y)*(target.y - next.y)));
						f = g + h * t;
						APoint ap1;
						ap1.p = next;
						ap1.g = g;
						ap1.f = f;
						openList.push_back(ap1);
						relation[next] = cur;
					}
					else{           //openList�д��ڱ����ڵ�
						g = ap.g + 1 + gn;
						if (g < openList[oi].g){
							openList[oi].g = g;
							h = abs(target.x - next.x) + abs(target.y - next.y);
							t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
								(target.y - next.y)*(target.y - next.y)));
							f = openList[oi].g + h * t;
							openList[oi].f = f;
							relation[next] = cur;
						}
					}
				}
			}
		}

		if (cur == target){
			flag = false;
			break;
		}
		sort(openList.begin(), openList.end());
	}

	vector<Point> result;

	result.push_back(target);
	Point cur = target;
	Point next = relation[cur];
	while (!(relation[cur] == source)){
		//relation[cur] = Avoid(relation[cur]);
		result.push_back(relation[cur]);
		cur = relation[cur];
	}

	result.push_back(source);
	
	return result.size();
}

vector<Point> 
Robot::ShortestPath(Point& source, Point& target){   // find a route from source to target
	
	vector<Point> path;

	if (workmap[source.x][source.y] == 1 || workmap[target.x][target.y] == 1){
		cout << "�����յ����ϰ���" << endl;
	}

	if (source == target){
		path.push_back(source);
		return path;
	}
	// A* algorithm

	struct APoint{
		Point p;
		double g;
		double f;
		bool operator < (const APoint &a) const {
			return f < a.f;  // ��Сֵ����
		}
	};

	vector<Point> closeList;
	vector<APoint> openList;
	map<Point, Point> relation;
	int choice = 0;
	APoint ap;
	ap.p = source;
	ap.g = 0;
	ap.f = 0;
	openList.push_back(ap);
	double g = 0;
	double h = 0;
	double f = 0;
	double t = 0;
	bool flag = true;
	while (openList.empty() == false && flag == true){
		ap = openList[0];
		openList.erase(openList.begin());
		Point cur = ap.p;
		//cur = Avoid(cur);
		closeList.push_back(cur);
		Point next;
		double gn = 0;


		for (int direct = 1; direct <= 4; ++direct){
			if (direct == 1){
				next = cur.up();
				gn = weightMap[ap.p.x][ap.p.y].up;
			}
			else if (direct == 2){
				next = cur.right();
				gn = weightMap[ap.p.x][ap.p.y].right;
			}
			else if (direct == 3){
				next = cur.down();
				gn = weightMap[ap.p.x][ap.p.y].down;
			}
			else if (direct == 4){
				next = cur.left();
				gn = weightMap[ap.p.x][ap.p.y].left;
			}
			if (next.x >= 0 && next.x < mapRowNumber && next.y >= 0 &&
				next.y < mapColumnNumber && workmap[next.x][next.y] == 0){
				//�����µ�F(N)

				int ci = -1;
				for (int i = 0; i < closeList.size(); ++i){
					if (closeList[i] == next){
						ci = i;
						break;
					}
				}

				if (ci == -1){
					int oi = -1;
					for (int i = 0; i < openList.size(); ++i){
						if (openList[i].p == next){
							oi = i;
							break;
						}
					}
					if (oi == -1){    //openList�в����ڱ����ڵ�
						g = ap.g + 1 + gn;
						h = abs(target.x - next.x) + abs(target.y - next.y);
						if (choice == 0){
							t = 1;
						}
						else if (choice == 1){
							t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
								(target.y - next.y)*(target.y - next.y)));
						}
						f = g + h * t;
						APoint ap1;
						ap1.p = next;
						ap1.g = g;
						ap1.f = f;
						openList.push_back(ap1);
						relation[next] = cur;
					}
					else{           //openList�д��ڱ����ڵ�
						g = ap.g + 1 + gn;
						if (g < openList[oi].g){
							openList[oi].g = g;
							h = abs(target.x - next.x) + abs(target.y - next.y);
							if (choice == 0){
								t = 1;
							}
							else if (choice == 1){
								t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
									(target.y - next.y)*(target.y - next.y)));
							}
							f = openList[oi].g + h * t;
							openList[oi].f = f;
							relation[next] = cur;
						}
					}
				}
			}
		}

		if ( cur == target){
			flag = false;
			break;
		}
		sort(openList.begin(),openList.end());
	}

	vector<Point> result;

	result.push_back(target);
	Point cur = target;
	Point next = relation[cur];
	while (!(relation[cur] == source)){
		result.push_back(relation[cur]);
		cur = relation[cur];
	}

	result.push_back(source);
	int low = 0, high = result.size() - 1;
	while (low < high){
		Point tmp = result[low];
		result[low] = result[high];
		result[high] = tmp;
		++low;
		--high;
	}
	return result;
}

vector<Point>
Robot::ShortestPath(Point& source, Point& target, Point Obs){   //this function it unused

	vector<Point> path;
	vector<vector<int>> tmpworkmap = workmap;
	tmpworkmap[Obs.x][Obs.y] = 1;
	if (tmpworkmap[source.x][source.y] == 1 || tmpworkmap[target.x][target.y] == 1){
		cout << "�����յ����ϰ���" << endl;
		return path;
	}

	if (source == target){
		path.push_back(source);
		return path;
	}
	// A* algorithm

	struct APoint{
		Point p;
		double g;
		double f;
		bool operator < (const APoint &a) const {
			return f < a.f;  // ��Сֵ����
		}
	};

	vector<Point> closeList;
	vector<APoint> openList;
	map<Point, Point> relation;
	int choice = 0;
	APoint ap;
	ap.p = source;
	ap.g = 0;
	ap.f = 0;
	openList.push_back(ap);
	double g = 0;
	double h = 0;
	double f = 0;
	double t = 0;
	bool flag = true;
	while (openList.empty() == false && flag == true){
		ap = openList[0];
		openList.erase(openList.begin());
		Point cur = ap.p;
		closeList.push_back(cur);
		Point next;
		double gn = 0;


		for (int direct = 1; direct <= 4; ++direct){
			if (direct == 1){
				next = cur.up();
				gn = weightMap[ap.p.x][ap.p.y].up;
			}
			else if (direct == 2){
				next = cur.right();
				gn = weightMap[ap.p.x][ap.p.y].right;
			}
			else if (direct == 3){
				next = cur.down();
				gn = weightMap[ap.p.x][ap.p.y].down;
			}
			else if (direct == 4){
				next = cur.left();
				gn = weightMap[ap.p.x][ap.p.y].left;
			}
			if (next.x >= 0 && next.x < mapRowNumber && next.y >= 0 &&
				next.y < mapColumnNumber && tmpworkmap[next.x][next.y] == 0){
				//�����µ�F(N)

				int ci = -1;
				for (int i = 0; i < closeList.size(); ++i){
					if (closeList[i] == next){
						ci = i;
						break;
					}
				}

				if (ci == -1){
					int oi = -1;
					for (int i = 0; i < openList.size(); ++i){
						if (openList[i].p == next){
							oi = i;
							break;
						}
					}
					if (oi == -1){    //openList�в����ڱ����ڵ�
						g = ap.g + 1 + gn;
						h = abs(target.x - next.x) + abs(target.y - next.y);
						if (choice == 0){
							t = 1;
						}
						else if (choice == 1){
							t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
								(target.y - next.y)*(target.y - next.y)));
						}
						f = g + h * t;
						APoint ap1;
						ap1.p = next;
						ap1.g = g;
						ap1.f = f;
						openList.push_back(ap1);
						relation[next] = cur;
					}
					else{           //openList�д��ڱ����ڵ�
						g = ap.g + 1 + gn;
						if (g < openList[oi].g){
							openList[oi].g = g;
							h = abs(target.x - next.x) + abs(target.y - next.y);
							if (choice == 0){
								t = 1;
							}
							else if (choice == 1){
								t = 1.0 + 1.0 / (sqrt(1.0 * (target.x - next.x)*(target.x - next.x) +
									(target.y - next.y)*(target.y - next.y)));
							}
							f = openList[oi].g + h * t;
							openList[oi].f = f;
							relation[next] = cur;
						}
					}
				}
			}
		}

		if (cur == target){
			flag = false;
			break;
		}
		sort(openList.begin(), openList.end());
	}

	vector<Point> result;

	result.push_back(target);
	Point cur = target;
	Point next = relation[cur];
	while (!(relation[cur] == source)){
		result.push_back(relation[cur]);
		cur = relation[cur];
	}

	result.push_back(source);
	int low = 0, high = result.size() - 1;
	while (low < high){
		Point tmp = result[low];
		result[low] = result[high];
		result[high] = tmp;
		++low;
		--high;
	}
	return result;
}
//int OffsetY = 0, OffsetX = 0;
vector<Robot> RobotPosition;
bool
Robot::Move(){               //The way robot move
	// ����滮�õ�·��Ϊ�գ������¹滮·��
	vector<Robot> robot;
	if (Status == 0){   // ���������״̬Ϊ0����˵����ǰ�����Ѿ�ִ�����
		;
	}
	else if (Status == 1){
		//targetPosition = doingTask[0].source;
		targetPosition = doingTask[0].source;
		//targetPosition = Avoid(targetPosition);
	}
	else if (Status == 2){
		//targetPosition = doingTask[0].target;
		targetPosition = doingTask[0].source;
		//targetPosition = Avoid(targetPosition);
	}

	if (planPath.empty() == true){
		planPath = ShortestPath(currentPosition, targetPosition);
	}

	if (planPath.empty() == false){
		//��ӻ������ƶ��߼�
		if (waitTime == -1){
			nextPosition = planPath.front();
			mu.lock();
			auto it = Robot::robotPosition.find(nextPosition);
			if (it == Robot::robotPosition.end() || it->second == Id ){
				DockMap[currentPosition.x][currentPosition.y] = 0;
				
				Robot::robotPosition.erase(currentPosition);
				//Avoid(nextPosition);
				//nextPosition = Robot::Avoid(nextPosition);
				Robot::robotPosition.insert(map<Point, int>::value_type(nextPosition, Id));//Id
				/*Point DockPoint = Dock(nextPosition);
				if (DockPoint.x != 0 && DockPoint.y != 0) {
					for (int i = 0; i < TaskNum; ++i) {
						if (RobotPosition[i].currentPosition.x == currentPosition.x + DockPoint.x &&
							RobotPosition[i].currentPosition.y == currentPosition.y - DockPoint.y) {
							currentPosition.LeaderNum = i;
							nextPosition.x = RobotPosition[i].currentPosition.x + DockPoint.x;
							nextPosition.y = RobotPosition[i].currentPosition.y - DockPoint.x;
							break;
						}
					}
				}*/
				AvoidClear(nextPosition);
				currentPosition = nextPosition;
				//Avoid(currentPosition);
				DockMap[currentPosition.x][currentPosition.y] = 2;
				
				planPath.erase(planPath.begin());
				++distance;
				alreadyWait = 0;
			}
			else{    // �����˷�����ײ
				fightTime++;
				waitTime = 3;//3
			}
			mu.unlock();
		}
		else if (waitTime > 0){
			nextPosition = planPath.front();
			
			mu.lock();
			auto it = Robot::robotPosition.find(nextPosition);
			if (it == Robot::robotPosition.end() || it->second == Id){
				DockMap[currentPosition.x][currentPosition.y] = 0;
				
				Robot::robotPosition.erase(currentPosition);
				//Avoid(nextPosition);
				Robot::robotPosition.insert(map<Point, int>::value_type(nextPosition, Id));//Id
				/*Point DockPoint = Dock(nextPosition);
				if (DockPoint.x != 0 && DockPoint.y != 0) {
					for (int i = 0; i < TaskNum; ++i) {
						if (RobotPosition[i].currentPosition.x == currentPosition.x + DockPoint.x &&
							RobotPosition[i].currentPosition.y == currentPosition.y - DockPoint.y) {
							currentPosition.LeaderNum = i;
							nextPosition.x = RobotPosition[i].currentPosition.x + DockPoint.x;
							nextPosition.y = RobotPosition[i].currentPosition.y - DockPoint.x;
							break;
						}
					}
				}*/
				AvoidClear(nextPosition);
				currentPosition = nextPosition;
				DockMap[currentPosition.x][currentPosition.y] = 2;
				//Avoid(currentPosition);
				planPath.erase(planPath.begin());
				++distance;
				waitTime = -1;
			}
			else{
				// ���¹滮·������һ������ѡ�����¹滮·��
				++alreadyWait;
				//nextPosition = Robot::Avoid(nextPosition);
				Point p = Point(nextPosition.x - currentPosition.x, nextPosition.y - currentPosition.y);
				if (p.x == -1 && p.y == 0){
					weightMap[currentPosition.x][currentPosition.y].up += alreadyWait;
				}
				else if (p.x == 1 && p.y == 0){
					weightMap[currentPosition.x][currentPosition.y].down += alreadyWait;
				}
				else if (p.x == 0 && p.y == -1){
					weightMap[currentPosition.x][currentPosition.y].left += alreadyWait;
				}
				else if (p.x == 0 && p.y == 1){
					weightMap[currentPosition.x][currentPosition.y].right += alreadyWait;
				}
				--waitTime;
			}
			mu.unlock();
		}
		else if (waitTime == 0){
			//���¹滮·��
			planPath = ShortestPath(currentPosition, targetPosition);
			waitTime = -1;
		}
		

	}
	if (currentPosition == targetPosition){
		if (Status == 1){
			Status = 2;
		}
		else if (Status == 2){
			doingTask.erase(doingTask.begin());//Jiaming remove
			if (doingTask.empty()){  //����������������Ϊ�գ���ѡ�񷵻����
				Status = 3;
				//targetPosition = initPosition;
			}
			else{    //ǰ�����
				if (doingTask.empty() == false){
					Status = 1;
				}
				else{
					Status = 3;
					//targetPosition = initPosition;
				}
			}
		}
		else if (Status == 3){
			Status = 0;
		}
	}
	return true;
}

bool RecordLog(string s){
	ofstream logFile;
	logFile.open("../TestRobot/Log.txt", ofstream::app);
	if (logFile){
		logFile << s.c_str() << endl;
		logFile.close();
	}
	else{
		cout << " Failed to open the target file . The target file is Log.txt. " << endl;
		return false;
	}
	return true;
}

int Robot::robotNumber = 0;
int targetRobot = -1, usedRobot = -1;
vector<int> RobotUsedFlag;
int usedFlag;
vector<float> Robot::bidTable;
mutex Robot::mu;
map<Point, int> Robot::robotPosition;

int main(){
	//����־�ļ�
	ofstream logFile;
	logFile.open("../TestRobot/Log.txt", ofstream::out);
	if (logFile){
		logFile << "System Init ......" << endl;
		logFile.close();
	}
	else
		cout << " Failed to open the target file . The target file is Log.txt. " << endl;
	cout << "System Start ......" << endl;
	int seedOfRandom = 8141106;     // �������
	srand(seedOfRandom);
	//cout << "The seed of the random is " << seedOfRandom << endl;
	RecordLog("The seed of the random is " + to_string(seedOfRandom));
	//�������������rule
	Rule rule;
	rule.ReadMap();
	rule.ReadRobotInitPosition();
	//����rule�����������
	/*rule.GenerateTask();
	return 0;*/

	int numberTime = 1;

	while (numberTime > 0){
		rule.ReadTask();//The first time to read Task file

		//Jiaming start from here
		//rule.Cut(TaskToDo);
		rule.ToDoTask = TaskToDo;
		//rule.ToDoTask = SingleSet;

		//for (int i = 0; i < rule.ToDoTask.size(); ++i) {
		//	rule.waitToAssigned.push_back(rule.ToDoTask[i]);
		//}

		int curTime = 0;
		rule.setGlobalTime(curTime);
		int robotNumber = rule.getRobotNumber();
		for (int i = 0; i < robotNumber; ++i) {
			usedFlag = 0;
			RobotUsedFlag.push_back(usedFlag);
		}
		Robot::bidTable.resize(robotNumber, INT_MAX);
		//����������
		vector<Robot> robot;
		for (int i = 0; i < robotNumber; ++i){
			Robot tmp;
			tmp.Id = i;
			tmp.initPosition = rule.getRobotInitPosition(i);
			tmp.currentPosition = rule.getRobotInitPosition(i);
			DockMap[tmp.currentPosition.x][tmp.currentPosition.y] = 2;
			tmp.targetPosition = rule.getRobotInitPosition(i);
			//tmp.targetPosition = Robot::Avoid(tmp.targetPosition);
			tmp.workmap = rule.getMap();
			tmp.mapRowNumber = rule.getMapRowNumber();
			tmp.mapColumnNumber = rule.getMapColumnNumber();
			tmp.weightMap = rule.getWeightMap();
			tmp.Status = 0;
			robot.push_back(tmp);
			rule.setRobotCurrentPosition(i, tmp.currentPosition);
			rule.setRobotTargetPosition(i, tmp.targetPosition);
		}
		RobotPosition = robot;
		rule.RecordCurrentAndTargetPosition();
		// ���ϲ�����ϵͳ�ĳ�ʼ������

		// ����������֪��ͼ
		for (int i = 0; i < robotNumber; ++i){
			string name = "weightMap";
			name = name + to_string(i);
			ifstream logFile;
			string Position = "../TestRobot/";
			Position = Position + name + ".txt";
			logFile.open(Position, istream::in);
			if (logFile){
				for (int r = 0; r < robot[i].mapRowNumber; ++r){
					for (int c = 0; c < robot[i].mapColumnNumber; ++c){
						logFile >> robot[i].weightMap[r][c].up
							>> robot[i].weightMap[r][c].down
							>> robot[i].weightMap[r][c].left
							>> robot[i].weightMap[r][c].right;
					}
				}
				logFile.close();
			}
		}

		// ϵͳ��ʼ����
		cout << "System Start......" << endl;
		RecordLog("System Start......");

		cout << "The number of Task is " << rule.ToDoTask.size() << endl;

		double overAllBeginTime = (double)clock();
		//getchar();


		int taskOrder = 0;
		//int AssignedFlag = 0;
		int AllRobotsStatus = 0;
		for (int i = 0; i < robotNumber; ++i){
			AllRobotsStatus += robot[i].Status;
		}

		int choice = 1;

		for (int i = 0; i < robotNumber; ++i){
			Robot::robotPosition.insert(map<Point, int>::value_type(robot[i].currentPosition, i));
		}//���������������λ��
		// �жϻ�����״̬����������
		while (AllFinishFlag == false) {
		//while (rule.ToDoTask.size() > 0 || robot[0].doingTask.size() > 0 || rule.waitToAssigned.size() > 0 || AllRobotsStatus > 0){    //ϵͳ��������
			if (rule.ToDoTask.size() > 0 || rule.waitToAssigned.size() > 0) {
				if (1) {
					//if (rule.ToDoTask.size() > 0 && rule.getGlobalTime() == rule.ToDoTask[0].publishTime){  //�����������ɵ�ʱ��
					if (choice == 0) {
						if (robot[taskOrder % robotNumber].Status == 3) {
							robot[taskOrder % robotNumber].planPath.clear();
						}
						if (robot[taskOrder % robotNumber].doingTask.empty() == true) {
							robot[taskOrder % robotNumber].Status = 1;
						}
						robot[taskOrder % robotNumber].doingTask.push_back(rule.ToDoTask[0]);
						rule.ToDoTask.erase(rule.ToDoTask.begin());
						++taskOrder;
					}
					else {
						if (rule.waitToAssigned.size() > 0) {
							rule.waitToAssigned.push_back(rule.ToDoTask[0]);
							rule.ToDoTask.erase(rule.ToDoTask.begin());
						}
						else {
							vector<thread> allthread;
							for (int i = 0; i < robotNumber; ++i) {
								allthread.push_back(thread(&Robot::Bid, std::ref(robot[i]), rule.ToDoTask[0]));
							}
							for (int i = 0; i < robotNumber; ++i) {
								allthread[i].join();
							}
							int lowestPrice = INT_MAX;
							//cout << targetRobot << endl;
							// ������ֻ����һ��Ŀ���
							for (int i = 0; i < robotNumber; ++i) {
								if (RobotUsedFlag[i] == 1) {
									continue;
								}
								if (Robot::bidTable[i] < lowestPrice) {
									lowestPrice = Robot::bidTable[i];
									targetRobot = i;
								}
							}
							RobotUsedFlag[targetRobot] = 1;
							if (lowestPrice == INT_MAX) {
								rule.waitToAssigned.push_back(rule.ToDoTask[0]);
								rule.ToDoTask.erase(rule.ToDoTask.begin());
							}
							else {
								if (robot[targetRobot].Status == 3) {
									robot[targetRobot].planPath.clear();
								}
								if (robot[targetRobot].doingTask.empty()) {
									robot[targetRobot].Status = 1;
								}
								robot[targetRobot].doingTask = robot[targetRobot].updateTask;
								rule.ToDoTask.erase(rule.ToDoTask.begin());
								for (int i = 0; i < robotNumber; ++i) {
									if (i != targetRobot) {
										robot[i].updateTask.clear();
									}
								}
							}
							Robot::bidTable.resize(robotNumber, INT_MAX);
						}
					}
				}
				if (rule.waitToAssigned.size() > 0) {
					vector<thread> allthread;
					for (int i = 0; i < robotNumber; ++i) {
						allthread.push_back(thread(&Robot::Bid, std::ref(robot[i]), rule.waitToAssigned[0]));
					}
					for (int i = 0; i < robotNumber; ++i) {
						allthread[i].join();
					}
					int targetRobot = -1, lowestPrice = INT_MAX;
					for (int i = 0; i < robotNumber; ++i) {
						if (Robot::bidTable[i] < lowestPrice) {
							lowestPrice = Robot::bidTable[i];
							targetRobot = i;
						}
					}
					if (lowestPrice == INT_MAX) {
						;
					}
					else {
						if (robot[targetRobot].Status == 3) {
							robot[targetRobot].planPath.clear();
						}
						if (robot[targetRobot].doingTask.empty()) {
							robot[targetRobot].Status = 1;
						}
						robot[targetRobot].doingTask = robot[targetRobot].updateTask;
						rule.waitToAssigned.erase(rule.waitToAssigned.begin());
						for (int i = 0; i < robotNumber; ++i) {
							if (i != targetRobot) {
								robot[i].updateTask.clear();
							}
						}
					}
					Robot::bidTable.resize(robotNumber, INT_MAX);
				}
			}
			//cout << "Here is the breaking point!" << endl;
			//getchar();
			if ((double)clock() - overAllBeginTime > 1000){   //һ������
				//getchar();

				for (int i = 0; i < robotNumber; ++i){
					cout << "robot " << i << ":";
					for (int j = 0; j < robot[i].doingTask.size(); ++j){
						cout << robot[i].doingTask[j].id << " ";
					}
					cout << endl;
					cout << robot[i].updateTask.size() << endl;
				}

				//getchar();

				// ִ������
				//if (AssignedFlag >= 4) {
					vector<thread> allthread;
					for (int i = 0; i < robotNumber; ++i) {
						//rule.Dock();
						allthread.push_back(thread(&Robot::Move, std::ref(robot[i])));//i
					}
					for (int i = 0; i < robotNumber; ++i) {
						allthread[i].join();
					}
					for (int i = 0; i < robotNumber; ++i) {
						//robot[i].Move();
						rule.setRobotCurrentPosition(i, robot[i].currentPosition);
						rule.setRobotTargetPosition(i, robot[i].targetPosition);
					}
					RobotPosition = robot;
					rule.RecordCurrentAndTargetPosition();
					AllRobotsStatus = 0;
					for (int i = 0; i < robotNumber; ++i) {
						AllRobotsStatus += robot[i].Status;
					}
					overAllBeginTime = (double)clock();
					rule.GlobalTime();
				//}
			}
			//if ((rule.ToDoTask.size() <= 0 && robot[0].doingTask.size() <= 0 && rule.waitToAssigned.size() <= 0 && AllRobotsStatus <= 0)) {
			//	cout << "Arrived at Current Destination" << endl;
			//	if (CombineSet.size() > 0) {
			//		//while (PublishNum.size() > 0) {
			//			Task ComToTask;
			//			for (int i = CombineSet.size(); i > CombineSet.size() - PublishNum.back(); --i) {
			//				ComToTask.id = CombineSet[i - 1].id;
			//				ComToTask.source.x = CombineSet[i - 1].source.x;
			//				ComToTask.source.y = CombineSet[i - 1].source.y;
			//				rule.ToDoTask.push_back(ComToTask);
			//			}
			//			for (int i = 0; i < PublishNum.back(); ++i) {
			//				CombineSet.pop_back();
			//			}
			//			PublishNum.pop_back();
			//			/*ComToTask.id = CombineSet[CombineSet.size() - 1].id;
			//			ComToTask.source.x = CombineSet[CombineSet.size() - 1].source.x;
			//			ComToTask.source.y = CombineSet[CombineSet.size() - 1].source.y;
			//			rule.ToDoTask.push_back(ComToTask);
			//			for (int i = CombineSet.size() - 2; i >= 0; --i) {
			//				if (CombineSet[i].source.x == CombineSet[i + 1].source.x) {
			//					ComToTask.id = CombineSet[i].id;
			//					ComToTask.source.x = CombineSet[i].source.x;
			//					ComToTask.source.y = CombineSet[i].source.y;
			//					rule.ToDoTask.push_back(ComToTask);
			//					CombineSet.pop_back();
			//				}
			//				else {
			//					CombineSet.pop_back();
			//					break;
			//				}
			//			}*/
			//			rule.PrintVector(rule.ToDoTask);
			//			cout << "----------------" << endl;
			//			rule.PrintVector(CombineSet);
			//			//Sleep(850);
			//			//break;
			//		//}
			//	}
			//	else {
			//		AllFinishFlag = true;
			//	}
			//}
			if (rule.ToDoTask.size() <= 0 && robot[0].doingTask.size() <= 0 && rule.waitToAssigned.size() <= 0 && AllRobotsStatus <= 0) {
				AllFinishFlag = true;
			}
		}
		//��ÿ�������˵���֪��ͼ���������Ա������֤
		for (int i = 0; i < robotNumber; ++i){
			string name = "weightMap";
			name = name + to_string(i);
			ofstream logFile;
			string Position = "../TestRobot/";
			Position = Position + name + ".txt";
			logFile.open(Position, ofstream::out);
			if (logFile){
				for (int r = 0; r < robot[i].mapRowNumber; ++r){
					for (int c = 0; c < robot[i].mapColumnNumber; ++c){
						logFile << robot[i].weightMap[r][c].up << " "
							<< robot[i].weightMap[r][c].down << " "
							<< robot[i].weightMap[r][c].left << " "
							<< robot[i].weightMap[r][c].right << endl;
					}
				}
				logFile.close();
			}
			else{
				cout << " Failed to open the target file . The target file is Log.txt. " << endl;
				return false;
			}
		}
		// ������
		int sumDis = 0;
		for (int i = 0; i < robotNumber; ++i){
			sumDis = sumDis + robot[i].distance;
		}
		cout << "sumDis=" << sumDis << endl;
		int sumFig = 0;
		for (int i = 0; i < robotNumber; ++i){
			cout << "Roobt " << i << " FightTime:" << robot[i].fightTime << endl;
			sumFig += robot[i].fightTime;
		}
		cout << "sumFig=" << sumFig << endl;
		ofstream logFile;
		logFile.open("../TestRobot/Result.txt", ofstream::app);
		if (logFile){
			logFile << sumFig << "  ";
			logFile << rule.globalTime << "  ";
			logFile << sumDis << endl;
			logFile.close();
		}
		else{
			cout << " Failed to open the target file . The target file is Log.txt. " << endl;
			return false;
		}
		--numberTime;
	}
	
	getchar();
}