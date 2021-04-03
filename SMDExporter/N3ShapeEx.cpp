/**
 * ______________________________________________________
 * This file is part of ko-smd-generator project.
 * 
 * @author       Mustafa Kemal Gılor <mustafagilor@gmail.com> (2016)
 * .
 * SPDX-License-Identifier:	MIT
 * ______________________________________________________
 */


#include "N3ShapeEx.h"
#include "types.h"
#include "DebugUtils.h"
#include <vector>


CN3ShapeEx::CN3ShapeEx()	{}
CN3ShapeEx::~CN3ShapeEx()	{}

void CN3ShapeEx::Load(FILE* fp)
{
	// TODO : Add ex support (currently supports only normal shape)
	#define KEY_VECTOR3 0
	#define KEY_QUATERNION 1
	m_ulOffset = ftell(fp);
	/// First step : Read file name
	uint32 iStrLen;
	fread_s(&iStrLen, 4, 4, 1, fp);
	std::vector<char> buffer(iStrLen + 1, NULL);
	fread_s(&(buffer[0]), iStrLen, 1, iStrLen, fp);
	std::string n(buffer.begin(), buffer.end());
	m_szName = n;
	/// Second step : Read object properties
	fread_s(&m_vPos, sizeof(__Vector3), sizeof(__Vector3), 1, fp);
	fread_s(&m_qRot, sizeof(__Quaternion), sizeof(__Quaternion), 1, fp);
	fread_s(&m_vScale, sizeof(__Vector3), sizeof(__Vector3), 1, fp);

	/// Third step : Load animkeys
	/*CN3Transform::Load -> animkey loads (keypos,keyrot,keyscale)*/
	for (int i = 0; i < 3; i++)
	{
		uint32 nCount;
		fread_s(&nCount, 4, 4, 1, fp);
		if(nCount >0)
		{
			uint32 iType;
			float fSamplingRate;
			fread_s(&iType, 4, 4, 1, fp);
			fread_s(&fSamplingRate, 4, 4, 1, fp);

			if(iType == KEY_VECTOR3)
			{
				__Vector3 * m_vecArr = new __Vector3[nCount];
				fread_s(m_vecArr, sizeof(__Vector3) * nCount, sizeof(__Vector3), nCount, fp);
				delete[] m_vecArr;
			}
			else if (iType == KEY_QUATERNION)
			{
				__Quaternion * m_quaternionArr = new __Quaternion[nCount];
				fread_s(m_quaternionArr, sizeof(__Quaternion) * nCount, sizeof(__Quaternion), nCount, fp);
				delete[] m_quaternionArr;
			}
			/// SMOOTH AS FUCK
		}
	}

	/// Fourth step : Read transform collision mesh filenames

	/// Transform collision Mesh filename 1
	fread_s(&iStrLen, 4, 4, 1, fp);
	ASSERT(iStrLen < 1000);
	if(iStrLen > 0)
	{
		// read mesh filename
		std::vector<char> mesh1(iStrLen + 1, NULL);
		fread_s(&(mesh1[0]), iStrLen, 1, iStrLen, fp);
		std::string meshName(mesh1.begin(), mesh1.end());
	}

	/// Transform collision Mesh filename 2
	fread_s(&iStrLen, 4, 4, 1, fp);
	ASSERT(iStrLen < 1000);
	if (iStrLen > 0)
	{
		// read mesh filename
		std::vector<char> mesh2(iStrLen + 1, NULL);
		fread_s(&(mesh2[0]), iStrLen, 1, iStrLen, fp);
		std::string meshName(mesh2.begin(), mesh2.end());
	}


	/// Fourth step : Read part count

	int32 partCount;
	fread_s(&partCount, 4, 4, 1, fp);
//	fseek(fp, 12, SEEK_CUR);

	/// Fifth step : Read each individual mesh
	for (int i = 0; i < partCount; i++)
	{
		//partMap.insert()
		/// SUB Step 1 : Read pivot
		__Vector3 vPivot;
		fread_s(&vPivot, sizeof(__Vector3), sizeof(__Vector3), 1, fp);

		/// SUB : Step2 -> Read mesh name
		fread_s(&iStrLen, 4, 4, 1, fp);
		std::vector<char> partNameBuffer(iStrLen + 1, NULL);
		fread_s(&(partNameBuffer[0]), iStrLen, 1, iStrLen, fp);
		std::string partName(partNameBuffer.begin(), partNameBuffer.end());

		/// SUB : Step3 -> Read material
		__Material mtMaterial;
		fread_s(&mtMaterial, sizeof(__Material), sizeof(__Material), 1, fp);

		uint32 nTextureCount;
		float fTexFPS;
		fread_s(&nTextureCount, 4, 4, 1, fp);
		fread_s(&fTexFPS, 4, 4, 1, fp);

		std::vector<std::string> textureNames;
		for (int j = 0; j < nTextureCount; j++)
		{
			fread_s(&iStrLen, 4, 4, 1, fp);
			if(iStrLen > 0)
			{
				std::vector<char> texNameBuffer(iStrLen + 1, NULL);
				fread_s(&(texNameBuffer[0]), iStrLen, 1, iStrLen, fp);
				std::string texName(texNameBuffer.begin(), texNameBuffer.end());
				textureNames.push_back(texName);
			}
		}
		partMap.insert(std::make_pair(partName, textureNames));
	}

	/// The actual useful part is here..
	fread_s(&m_iBelong, 4, 4, 1, fp);
	fread_s(&m_iEventID, 4, 4, 1, fp);
	fread_s(&m_iEventType, 4, 4, 1, fp);
	fread_s(&m_iNpcID, 4, 4, 1, fp);
	fread_s(&m_iNpcStatus, 4, 4, 1, fp);
	
	/// DONE

}