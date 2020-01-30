#pragma once
// ��־��¼�洢��Log.txt��
#include <iostream>
#include <fstream> 

using namespace std;

bool RecordLog(string s) {
	ofstream logFile;
	logFile.open("../TestRobot/Log.txt", ofstream::app);
	if (logFile) {
		logFile << s.c_str() << endl;
		logFile.close();
	}
	else {
		cout << " Failed to open the target file . The target file is Log.txt. " << endl;
		return false;
	}
	return true;
}