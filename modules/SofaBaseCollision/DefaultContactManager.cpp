/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 MGH, INRIA, USTL, UJF, CNRS, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaBaseCollision/DefaultContactManager.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/core/objectmodel/Tag.h>
#include <sofa/helper/AdvancedTimer.h>

namespace sofa
{

namespace component
{

namespace collision
{

SOFA_DECL_CLASS(DefaultContactManager)

int DefaultContactManagerClass = core::RegisterObject("Default class to create reactions to the collisions")
        .add< DefaultContactManager >()
        .addAlias("CollisionResponse")
        ;

DefaultContactManager::DefaultContactManager()
    : response(initData(&response, "response", "contact response class"))
    , responseParams(initData(&responseParams, "responseParams", "contact response parameters (syntax: name1=value1&name2=value2&...)"))
{
}

DefaultContactManager::~DefaultContactManager()
{
    // Contacts are now attached to the graph.
    // So they will be deleted by the DeleteVisitor
    // FIX crash on unload bug. -- J. Allard
    //clear();
}

sofa::helper::OptionsGroup DefaultContactManager::initializeResponseOptions(core::collision::Pipeline *pipeline)
{
    helper::set<std::string> listResponse;
    if (pipeline) listResponse=pipeline->getResponseList();
    else
    {
        core::collision::Contact::Factory::iterator it;
        for (it=core::collision::Contact::Factory::getInstance()->begin(); it!=core::collision::Contact::Factory::getInstance()->end(); ++it)
        {
            listResponse.insert(it->first);
        }
    }
    sofa::helper::OptionsGroup responseOptions(listResponse);
    if (listResponse.find("default") != listResponse.end())
        responseOptions.setSelectedItem("default");
    return responseOptions;
}

void DefaultContactManager::init()
{
    if (response.getValue().size() == 0)
    {
        core::collision::Pipeline *pipeline=static_cast<simulation::Node*>(getContext())->collisionPipeline;
        response.setValue(initializeResponseOptions(pipeline));
    }
}

void DefaultContactManager::reset()
{
    // Remove all previously created contacts and responses (same as cleanup)
    cleanup();
}

void DefaultContactManager::cleanup()
{
    // Remove all previously created contacts and responses
    for (sofa::helper::vector<core::collision::Contact::SPtr>::iterator it=contacts.begin(); it!=contacts.end(); ++it)
    {
        (*it)->removeResponse();
        (*it)->cleanup();
        //delete *it;
        it->reset();
    }
    contacts.clear();
    contactMap.clear();
}

void DefaultContactManager::createContacts(const DetectionOutputMap& outputsMap)
{
    using core::CollisionModel;
    using core::collision::Contact;

    int nbContact = 0;

    sofa::helper::AdvancedTimer::stepBegin("ProcessCurrentContacts");

	// First iterate on the collision detection outputs and look for existing or new contacts
	for (DetectionOutputMap::const_iterator outputsIt = outputsMap.begin(),
		outputsItEnd = outputsMap.end(); outputsIt != outputsItEnd ; ++outputsIt)
	{
		std::pair<ContactMap::iterator,bool> contactInsert =
			contactMap.insert(ContactMap::value_type(outputsIt->first,Contact::SPtr()));
		ContactMap::iterator contactIt = contactInsert.first;
		if (contactInsert.second)
		{
            // new contact
            //sout << "Creation new "<<contacttype<<" contact"<<sendl;
            CollisionModel* model1 = outputsIt->first.first;
            CollisionModel* model2 = outputsIt->first.second;
            std::string responseUsed = getContactResponse(model1, model2);

            // We can create rules in order to not respond to specific collisions
            if (!responseUsed.compare("null"))
            {
				contactMap.erase(contactIt);
            }
			else
			{
				Contact::SPtr contact = Contact::Create(responseUsed, model1, model2, intersectionMethod,
					this->f_printLog.getValue());

                if (contact == nullptr)
				{
					std::string model1class = model1->getClassName();
					std::string model2class = model2->getClassName();
					int count = ++errorMsgCount[std::make_pair(responseUsed,
						std::make_pair(model1class, model2class))];
					if (count <= 10)
					{
						serr << "Contact " << responseUsed << " between " << model1->getClassName()
							<< " and " << model2->getClassName() << " creation failed" << sendl;
						if (count == 1)
						{
							serr << "Supported models for contact " << responseUsed << ":" << sendl;
							for (Contact::Factory::const_iterator it =
								Contact::Factory::getInstance()->begin(),
								itend = Contact::Factory::getInstance()->end(); it != itend; ++it)
							{
								if (it->first != responseUsed) continue;
								serr << "   " << helper::gettypename(it->second->type()) << sendl;
							}
							serr << sendl;
						}
						if (count == 10) serr << "further messages suppressed" << sendl;
					}
					contactMap.erase(contactIt);
				}
				else
				{
					contactIt->second = contact;
					contact->setName(model1->getName() + std::string("-") + model2->getName());
					setContactTags(model1, model2, contact);
					contact->f_printLog.setValue(this->f_printLog.getValue());
					contact->init();
                    contact->setDetectionOutputs(outputsIt->second.first);
					++nbContact;
				}
			}
		}
		else
		{
            contactIt->second->setDetectionOutputs(outputsIt->second.first);
            ++nbContact;
		}
	}

    sofa::helper::AdvancedTimer::stepNext("ProcessCurrentContacts","RemoveInactiveContacts");


	// Then look at previous contacts
	// and remove inactive contacts
    inactiveContacts.clear();

	for (ContactMap::iterator contactIt = contactMap.begin(), contactItEnd = contactMap.end();
		contactIt != contactItEnd;)
	{
		bool remove = false;
		DetectionOutputMap::const_iterator outputsIt = outputsMap.find(contactIt->first);
		if (outputsIt == outputsMap.end())
		{
            // inactive contact
            if (contactIt->second->keepAlive())
            {
                contactIt->second->setDetectionOutputs(nullptr);
                ++nbContact;
            }
            else
            {
				remove = true;
			}
		}
		if (remove)
		{
			if (contactIt->second)
			{
                inactiveContacts.push_back(contactIt->second);
			}
			ContactMap::iterator eraseIt = contactIt;
			++contactIt;

			contactMap.erase(eraseIt);

		}
		else
		{
			++contactIt;
		}
	}

    // now update contactVec
    sofa::helper::AdvancedTimer::stepNext("RemoveInactiveContacts", "UpdateContactVector");
    contacts.clear();
    contacts.reserve(nbContact);
    for (ContactMap::const_iterator contactIt = contactMap.begin(), contactItEnd = contactMap.end();
		contactIt != contactItEnd; ++contactIt)
    {
        contacts.push_back(contactIt->second);
    }

    sofa::helper::AdvancedTimer::stepEnd("UpdateContactVector");
}

std::string DefaultContactManager::getContactResponse(core::CollisionModel* model1, core::CollisionModel* model2)
{
    std::string responseUsed = response.getValue().getSelectedItem();
    std::string params = responseParams.getValue();
    if (!params.empty())
    {
        responseUsed += '?';
        responseUsed += params;
    }
    std::string response1 = model1->getContactResponse();
    std::string response2 = model2->getContactResponse();

    if (response1.empty()  &&  response2.empty()) return responseUsed;
    if (response1.empty()  && !response2.empty()) return response2;
    if (!response1.empty() &&  response2.empty()) return response1;

    if (response1 != response2) return responseUsed;
    else return response1;
}

void DefaultContactManager::draw(const core::visual::VisualParams* vparams)
{
    for (sofa::helper::vector<core::collision::Contact::SPtr>::iterator it = contacts.begin(); it!=contacts.end(); ++it)
    {
        if ((*it)!=nullptr)
            (*it)->draw(vparams);
    }
}

void DefaultContactManager::removeContacts(ContactVector &c)
{
    for (ContactVector::iterator contactIt = c.begin(); contactIt != c.end(); contactIt++)
    {
        (*contactIt)->removeResponse();
        (*contactIt)->cleanup();
        (*contactIt).reset();
    }
}

void DefaultContactManager::setContactTags(core::CollisionModel* model1, core::CollisionModel* model2, core::collision::Contact::SPtr contact)
{
    sofa::core::objectmodel::TagSet tagsm1 = model1->getTags();
    sofa::core::objectmodel::TagSet tagsm2 = model2->getTags();
    sofa::core::objectmodel::TagSet::iterator it;
    for(it=tagsm1.begin(); it != tagsm1.end(); ++it)
        contact->addTag(*it);
    for(it=tagsm2.begin(); it!=tagsm2.end(); ++it)
        contact->addTag(*it);
}

} // namespace collision

} // namespace component

} // namespace sofa
