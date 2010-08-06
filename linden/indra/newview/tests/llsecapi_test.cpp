/** 
 * @file llsecapi_test.cpp
 * @author Roxie
 * @date 2009-02-10
 * @brief Test the sec api functionality
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */
#include "../llviewerprecompiledheaders.h"
#include "../llviewernetwork.h"
#include "../test/lltut.h"
#include "../llsecapi.h"
#include "../llsechandler_basic.h"
#include "../../llxml/llcontrol.h"


//----------------------------------------------------------------------------               
// Mock objects for the dependencies of the code we're testing                               

LLControlGroup::LLControlGroup(const std::string& name)
: LLInstanceTracker<LLControlGroup, std::string>(name) {}
LLControlGroup::~LLControlGroup() {}
BOOL LLControlGroup::declareString(const std::string& name,
                                   const std::string& initial_val,
                                   const std::string& comment,
                                   BOOL persist) {return TRUE;}
void LLControlGroup::setString(const std::string& name, const std::string& val){}
std::string LLControlGroup::getString(const std::string& name)
{
	return "";
}


LLControlGroup gSavedSettings("test");

LLSecAPIBasicHandler::LLSecAPIBasicHandler() {}
void LLSecAPIBasicHandler::init() {}
LLSecAPIBasicHandler::~LLSecAPIBasicHandler() {}
LLPointer<LLCertificate> LLSecAPIBasicHandler::getCertificate(const std::string& pem_cert) { return NULL; }
LLPointer<LLCertificate> LLSecAPIBasicHandler::getCertificate(X509* openssl_cert) { return NULL; }
LLPointer<LLCertificateChain> LLSecAPIBasicHandler::getCertificateChain(const X509_STORE_CTX* chain) { return NULL; }
LLPointer<LLCertificateStore> LLSecAPIBasicHandler::getCertificateStore(const std::string& store_id) { return NULL; }
void LLSecAPIBasicHandler::setProtectedData(const std::string& data_type, const std::string& data_id, const LLSD& data) {}
LLSD LLSecAPIBasicHandler::getProtectedData(const std::string& data_type, const std::string& data_id) { return LLSD(); }
void LLSecAPIBasicHandler::deleteProtectedData(const std::string& data_type, const std::string& data_id) {}
LLPointer<LLCredential> LLSecAPIBasicHandler::createCredential(const std::string& grid, const LLSD& identifier, const LLSD& authenticator) { return NULL; }
LLPointer<LLCredential> LLSecAPIBasicHandler::loadCredential(const std::string& grid) { return NULL; }
void LLSecAPIBasicHandler::saveCredential(LLPointer<LLCredential> cred, bool save_authenticator) {}
void LLSecAPIBasicHandler::deleteCredential(LLPointer<LLCredential> cred) {}

// -------------------------------------------------------------------------------------------
// TUT
// -------------------------------------------------------------------------------------------
namespace tut
{
	// Test wrapper declaration : wrapping nothing for the moment
	struct secapiTest
	{
		
		secapiTest()
		{
		}
		~secapiTest()
		{
		}
	};
	
	// Tut templating thingamagic: test group, object and test instance
	typedef test_group<secapiTest> secapiTestFactory;
	typedef secapiTestFactory::object secapiTestObject;
	tut::secapiTestFactory tut_test("llsecapi");
	
	// ---------------------------------------------------------------------------------------
	// Test functions 
	// ---------------------------------------------------------------------------------------
	// registration
	template<> template<>
	void secapiTestObject::test<1>()
	{
		// retrieve an unknown handler

		ensure("'Unknown' handler should be NULL", !(BOOL)getSecHandler("unknown"));
		LLPointer<LLSecAPIHandler> test1_handler =  new LLSecAPIBasicHandler();
		registerSecHandler("sectest1", test1_handler);
		ensure("'Unknown' handler should be NULL", !(BOOL)getSecHandler("unknown"));
		LLPointer<LLSecAPIHandler> retrieved_test1_handler = getSecHandler("sectest1");
		ensure("Retrieved sectest1 handler should be the same", 
			   retrieved_test1_handler == test1_handler);
		
		// insert a second handler
		LLPointer<LLSecAPIHandler> test2_handler =  new LLSecAPIBasicHandler();
		registerSecHandler("sectest2", test2_handler);
		ensure("'Unknown' handler should be NULL", !(BOOL)getSecHandler("unknown"));
		retrieved_test1_handler = getSecHandler("sectest1");
		ensure("Retrieved sectest1 handler should be the same", 
			   retrieved_test1_handler == test1_handler);

		LLPointer<LLSecAPIHandler> retrieved_test2_handler = getSecHandler("sectest2");
		ensure("Retrieved sectest1 handler should be the same", 
			   retrieved_test2_handler == test2_handler);
		
	}
}
