#pragma once
#include <Windows.h>
#include "Python.h"
#include "arrayobject.h"
#include "ndarrayobject.h"
#include "npy_common.h"
#include "utility.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION


class PythonPlot {
	public:
		PythonPlot(std::string python_script_name = "draw_image");
		~PythonPlot();
		
		void SetPySysPath();
		void SetScriptName(void);

		void CopyRawImageData(BYTE * pRawImageData, int image_length);
		void CopyRawImageData(BYTE * pRawImageData, int width, int height);
		int CopyArrayToNumpyArray();
		void SendNumpyArrayToPythonFunction();

		//int DrawPlot(std::string func_name = "plot_image");
		int DrawPlot(std::string func_name = "save_image_by_numpy");

		void InitPythonThread();
		int FinalizePythonThread();

		int isCallableObject();
		int isValidReturnFromPythonFunction();

		void errPrintCallableObject();
		void errPrintValidReturn();

	private:
		std::string python_script_name_;
		std::string python_file_name_;

		PyObject *pName, *pModule, *pFunc;
		PyObject *pArgs, *pValue;
		PyObject *pVec, *pList;
		PyObject *pWidth, *pHeight;


		int kIsInitialized;
		int kIsImageDataCopied;

		BYTE * pImageData_;

		int image_length_;
		int image_width_;
		int image_height_;

		int test;
		PyThreadState * main_state_;

		npy_intp vdim_[1];
};

struct NodataCopiedError {};
struct ModuleNotCallableError {};