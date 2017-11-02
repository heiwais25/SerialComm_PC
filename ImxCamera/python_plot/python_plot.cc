#include "stdafx.h"
#include "python_plot.h"

// Todo
// 1. Cleaning up the numpy including statement
// 2. How to add finalize step : it is necessary for prevent memory leakage
// 3. Sometimes, there are falut in 'SetPySysPath' function in local debuggin. reason?


// It is basically set to already made python script
PythonPlot::PythonPlot(std::string python_script_name) {
	python_script_name_ = python_script_name;
	kIsImageDataCopied = false;
	
	//kIsInitialized = false;
	
}

PythonPlot::~PythonPlot() {

}

// In the case of starting this python embedding process, we need to start with this function
// 1. Py_Initialize
// 2. Set py sys path to current directory(It can change depending on user setting)
// 3. Set script name which is located in set directory(It also can change depending on user setting)
void PythonPlot::InitPythonThread() {
	Py_Initialize();
	SetPySysPath();
	try {
		SetScriptName();
	}
	catch (...) {
		std::cerr << "Failed to load " << python_script_name_.c_str() << std::endl;
		exit(1);
	}
	kIsInitialized = true;
}

// Set directory information to py sys to make visual studio can find the python script
// Todo : sysPath, curDir을 언제 free?
void PythonPlot::SetPySysPath() {
	PyObject* sysPath = PySys_GetObject((char*)"path");
	PyObject* curDir = PyUnicode_FromString("./python_plot");
	PyList_Append(sysPath, curDir);
}

// Depending on user set python script, import that python script
void PythonPlot::SetScriptName() {
	pName = PyUnicode_DecodeFSDefault(python_script_name_.c_str());
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule == NULL) {
		PyErr_Print();
		throw ModuleNotCallableError{};
	}
}

// Copy raw data from image processing class or other
void PythonPlot::CopyRawImageData(BYTE * pRawImageData, int image_length) {
	pImageData_ = pRawImageData;
	image_length_ = image_length;
	vdim_[0] = image_length_;
	kIsImageDataCopied = true;
}

// Draw plot : It is main stream code
// 1. Init python thread
// 2. Check valid function name
// 3. Copy Array to numpy array
int PythonPlot::DrawPlot(std::string func_name){
	python_file_name_ = func_name;

	// It needs to initialize every time
	InitPythonThread();

	if (!kIsImageDataCopied) {
		std::cerr << "Needs to copy image data first" << std::endl;
		throw NodataCopiedError{};
	}

	// Input valid function name and check 
	pFunc = PyObject_GetAttrString(pModule, func_name.c_str()); // Call function from module
	if (!isCallableObject()) {
		errPrintCallableObject();
		return -1;
	} 
			
	// Process to send array convert to numpy
	CopyArrayToNumpyArray();
	SendNumpyArrayToPythonFunction();
	if (!isValidReturnFromPythonFunction()) {
		errPrintValidReturn();
		return 1;
	}

	// Check result from function and save it
	std::cout << "Result of call: " << PyLong_AsLong(pValue) << std::endl;
	Py_DECREF(pValue);
	Py_XDECREF(pFunc);
	Py_DECREF(pModule);

	return 0;
}


// 앞의 전체적인 process가 돌아간 경우에만 사용한다.
// kInitialized를 지속적으로 check해야함
// 이 함수는 마지막에만 호출
int PythonPlot::FinalizePythonThread(void) {
	kIsInitialized = false;
	kIsImageDataCopied = false;
	Py_Finalize();
	return 0;
	/*if (Py_FinalizeEx() < 0) 
		return 120;*/
}

/*--------------------------------------------------------------------------------------------------------------

Helper function

--------------------------------------------------------------------------------------------------------------*/
int PythonPlot::isCallableObject() {
	return pFunc && PyCallable_Check(pFunc);
}

int PythonPlot::isValidReturnFromPythonFunction() {
	return pValue != NULL;
}

// Print error statement when there are error on callalbe object
void PythonPlot::errPrintCallableObject() {
	if (PyErr_Occurred())
		PyErr_Print();
	std::cerr << "Cannot find function " << python_file_name_.c_str() << std::endl;
}

void PythonPlot::errPrintValidReturn(){
	Py_DECREF(pFunc);
	Py_DECREF(pModule);
	PyErr_Print();
	std::cerr << "Call failed" << std::endl;
}

int PythonPlot::CopyArrayToNumpyArray() {
	import_array();
	pVec = PyArray_SimpleNewFromData(1, vdim_, PyArray_UINT8, pImageData_); // Is it okay to not using Py_DECREF
}

void PythonPlot::SendNumpyArrayToPythonFunction() {
	pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, pVec);
	pValue = PyObject_CallObject(pFunc, pArgs);
	Py_DECREF(pArgs);
}