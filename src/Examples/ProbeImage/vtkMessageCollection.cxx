/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Lee Newberg, Kitware Inc.

==============================================================================*/


#include "vtkMessageCollection.h"

// MRML includes
#include "vtkCommand.h"

namespace
{
  // This message type is chosen arbitrary and used only inside this class
  // (this event is chosen because it has a name that is remotely related
  // to separation between groups of messages, and unlikely to be used otherwise).
  const int SEPARATOR_MESSAGE_TYPE = vtkCommand::PropertyModifiedEvent;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMessageCollection);

//----------------------------------------------------------------------------
void
vtkMessageCollection
::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Messages: " << &this->Messages << "\n";
  for (int i = 0; i < this->GetNumberOfMessages(); i++)
    {
    const unsigned long messageType = this->GetNthMessageType(i);
    const std::string messageText = this->GetNthMessageText(i);
    os << indent << "MessagesMember: " << messageType << " "
       << messageText << "\n";
    }
}

//----------------------------------------------------------------------------
vtkMessageCollection::Message
::Message(unsigned long messageType, const std::string &messageText)
  : MessageType(messageType), MessageText(messageText)
{
}

//----------------------------------------------------------------------------
int
vtkMessageCollection
::GetNumberOfMessages() const
{
  return this->Messages.size();
}

//----------------------------------------------------------------------------
int
vtkMessageCollection
::GetNumberOfMessagesOfType(unsigned long messageType) const
{
  int response = 0;
  for (int i = 0; i < static_cast<int>(this->Messages.size()); ++i)
    {
    if (this->GetNthMessageType(i) == messageType)
      {
      ++response;
      }
    }
  return response;
}

//----------------------------------------------------------------------------
int
vtkMessageCollection
::GetNumberOfMessagesOfType(const char *eventName) const
{
  return GetNumberOfMessagesOfType(vtkCommand::GetEventIdFromString(eventName));
}

//----------------------------------------------------------------------------
unsigned long
vtkMessageCollection
::GetNthMessageType(int index) const
{
  return this->Messages.at(index).MessageType;
}

//----------------------------------------------------------------------------
std::string
vtkMessageCollection
::GetNthMessageText(int index) const
{
  return this->Messages.at(index).MessageText;
}

//----------------------------------------------------------------------------
void
vtkMessageCollection
::AddMessage(unsigned long messageType, const std::string &messageText)
{
  this->Messages.push_back({messageType, messageText});
}

//----------------------------------------------------------------------------
void vtkMessageCollection::AddSeparator()
{
  this->AddMessage(SEPARATOR_MESSAGE_TYPE, "\n--------\n");
}

//----------------------------------------------------------------------------
void
vtkMessageCollection
::ClearMessages()
{
  this->Messages.clear();
}

//----------------------------------------------------------------------------
vtkMessageCollection::vtkMessageCollection()
{
  this->CallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallbackCommand->SetCallback(vtkMessageCollection::CallbackFunction);
  this->CallbackCommand->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkMessageCollection::~vtkMessageCollection()
{
  this->SetObservedObject(nullptr);
}

//----------------------------------------------------------------------------
void vtkMessageCollection::DeepCopy(vtkMessageCollection* source)
{
  if (!source)
    {
    vtkErrorMacro("vtkMessageCollection::DeepCopy failed: invalid source");
    return;
    }
  this->Messages = source->Messages;
}

//----------------------------------------------------------------------------
void vtkMessageCollection::AddMessages(vtkMessageCollection* source, const std::string& prefix)
{
  if (!source)
    {
    vtkErrorMacro("vtkMessageCollection::AddMessages failed: invalid source");
    return;
    }
  for (int i = 0; i < source->GetNumberOfMessages(); i++)
    {
    this->AddMessage(source->GetNthMessageType(i), prefix + source->GetNthMessageText(i));
    }
}

//----------------------------------------------------------------------------
std::string vtkMessageCollection::GetAllMessagesAsString(bool* errorFoundPtr/*=nullptr*/, bool* warningFoundPtr/*=nullptr*/)
{
  std::string messagesStr;
  bool warningFound = false;
  bool errorFound = false;

  // Check if we need to display bullet-point list
  // (yes, if there are at least two non-separator messages)
  bool showAsBulletPointList = false;
  int numberOfNonSeparatorMessages = 0;
  const int numberOfMessages = this->GetNumberOfMessages();
  for (int index = 0; index < numberOfMessages; ++index)
    {
    const unsigned long messageType = this->GetNthMessageType(index);
    if (messageType != SEPARATOR_MESSAGE_TYPE)
      {
      numberOfNonSeparatorMessages++;
      }
    if (numberOfNonSeparatorMessages >= 2)
      {
      showAsBulletPointList = true;
      break;
      }
    }

  // There is at least one message from the storage node.
  for (int index = 0; index < numberOfMessages; ++index)
    {
    const unsigned long messageType = this->GetNthMessageType(index);
    const std::string messageText = this->GetNthMessageText(index);
    if (messageType == SEPARATOR_MESSAGE_TYPE)
      {
      // do not print separator at the end of the message list
      if (index == numberOfMessages - 1)
        {
        continue;
        }
      }
    else if (!messageText.empty() && showAsBulletPointList)
      {
      messagesStr += "- ";
      }
    switch (messageType)
      {
    case vtkCommand::WarningEvent:
      warningFound = true;
      if (!messageText.empty())
        {
        messagesStr.append("Warning: ");
        }
      break;
    case vtkCommand::ErrorEvent:
      errorFound = true;
      if (!messageText.empty())
        {
        messagesStr.append("Error: ");
        }
      break;
      }
    if (!messageText.empty())
      {
      messagesStr.append(messageText.c_str());
      messagesStr.append("\n");
      }
    }

  if (errorFoundPtr)
    {
    *errorFoundPtr = errorFound;
    }
  if (warningFoundPtr)
    {
    *warningFoundPtr = warningFound;
    }
  return messagesStr;
}

//----------------------------------------------------------------------------
void vtkMessageCollection::CallbackFunction(vtkObject* vtkNotUsed(caller),
  long unsigned int eventId, void* clientData, void* callData)
{
  vtkMessageCollection* self = reinterpret_cast<vtkMessageCollection*>(clientData);
  if (!self || !callData)
    {
    return;
    }
  std::string msg = reinterpret_cast<char*>(callData);
  const std::string chars = "\t\n\v\f\r ";
  msg.erase(msg.find_last_not_of(chars) + 1);
  self->AddMessage(eventId, msg);
}

//----------------------------------------------------------------------------
void vtkMessageCollection::SetObservedObject(vtkObject* observedObject)
{
  if (observedObject == this->ObservedObject)
    {
    // no change
    return;
    }
  if (this->ObservedObject)
    {
    this->ObservedObject->RemoveObservers(vtkCommand::ErrorEvent, this->CallbackCommand);
    this->ObservedObject->RemoveObservers(vtkCommand::WarningEvent, this->CallbackCommand);
    }
  this->ObservedObject = observedObject;
  if (this->ObservedObject)
    {
    this->ObservedObject->AddObserver(vtkCommand::ErrorEvent, this->CallbackCommand);
    this->ObservedObject->AddObserver(vtkCommand::WarningEvent, this->CallbackCommand);
    }
}
