/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <string>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>
#include <alljoyn/Observer.h>

#define INTF_NAME "com.example.Door"

using namespace std;
using namespace ajn;
using namespace qcc;

/* convenience class that hides all the marshalling boilerplate from sight */
class DoorProxy {
    ManagedProxyBusObject proxy;
    BusAttachment& bus;
  public:
    DoorProxy(ManagedProxyBusObject proxy, BusAttachment& bus) : proxy(proxy), bus(bus) { }

    bool IsValid() {
        return proxy->IsValid();
    }

    QStatus IsOpen(bool& val) {
        MsgArg value;
        QStatus status = proxy->GetProperty(INTF_NAME, "IsOpen", value);
        if (ER_OK == status) {
            status = value.Get("b", &val);
        }
        return status;
    }

    QStatus GetLocation(string& val) {
        MsgArg value;
        QStatus status = proxy->GetProperty(INTF_NAME, "Location", value);
        if (ER_OK == status) {
            char* cstr;
            status = value.Get("s", &cstr);
            if (ER_OK == status) {
                val = string(cstr);
            }
        }
        return status;
    }

    QStatus GetKeyCode(uint32_t& val) {
        MsgArg value;
        QStatus status = proxy->GetProperty(INTF_NAME, "KeyCode", value);
        if (ER_OK == status) {
            status = value.Get("u", &val);
        }
        return status;
    }

    QStatus GetAllProperties(bool& isOpen, string& location, uint32_t& keyCode) {
        QStatus status = IsOpen(isOpen);
        if (ER_OK == status) {
            status = GetLocation(location);
            if (ER_OK == status) {
                status = GetKeyCode(keyCode);
            }
        }
        return status;
    }

    QStatus Open() {
        Message reply(bus);
        QStatus status = proxy->MethodCall(INTF_NAME, "Open", NULL, 0, reply);
        return status;
    }

    QStatus Close() {
        Message reply(bus);
        QStatus status = proxy->MethodCall(INTF_NAME, "Close", NULL, 0, reply);
        return status;
    }

    QStatus KnockAndRun() {
        QStatus status = proxy->MethodCall(INTF_NAME, "KnockAndRun", NULL, 0);
        return status;
    }
};

static void help()
{
    cout << "q             quit" << endl;
    cout << "l             list all discovered doors" << endl;
    cout << "o <location>  open door at <location>" << endl;
    cout << "c <location>  close door at <location>" << endl;
    cout << "k <location>  knock-and-run at <location>" << endl;
    cout << "h             display this help message" << endl;
}

static void list_doors(BusAttachment& bus, Observer* observer)
{
    ManagedProxyBusObject proxy = observer->GetFirst();
    for (; proxy->IsValid(); proxy = observer->GetNext(proxy)) {
        DoorProxy door(proxy, bus);
        string location;
        bool isOpen;
        QStatus status = door.IsOpen(isOpen);
        if (ER_OK != status) {
            cerr << "Could not get IsOpen property for object " << proxy->GetUniqueName() << ":" << proxy->GetPath() << endl;
            continue;
        }
        status = door.GetLocation(location);
        if (ER_OK != status) {
            cerr << "Could not get Location property for object " << proxy->GetUniqueName() << ":" << proxy->GetPath() << endl;
            continue;
        }
        cout << "Door location: " << location << " open: " << isOpen << endl;
    }
}

static DoorProxy get_door_at_location(BusAttachment& bus, Observer* observer, const string& find_location)
{
    ManagedProxyBusObject proxy = observer->GetFirst();
    for (; proxy->IsValid(); proxy = observer->GetNext(proxy)) {
        DoorProxy door(proxy, bus);
        string location;
        QStatus status = door.GetLocation(location);
        if (ER_OK != status) {
            cerr << "Could not get Location property for object " << proxy->GetUniqueName() << ":" << proxy->GetPath() << endl;
            continue;
        }
        if (location == find_location) {
            return door;
        }
    }

    /* not found, return an invalid door proxy */
    return DoorProxy(proxy, bus);
}

static void open_door(BusAttachment& bus, Observer* observer, const string& location)
{
    DoorProxy door = get_door_at_location(bus, observer, location);

    if (door.IsValid()) {
        QStatus status = door.Open();

        if (ER_OK == status) {
            /* No error */
            cout << "Opening of door succeeded" << endl;
        } else if (ER_BUS_REPLY_IS_ERROR_MESSAGE == status) {
            /* MethodReply Error received (an error string) */
            cout << "Opening of door @ location " << location
                 << " returned an error." << endl;
        } else {
            /* Framework error or MethodReply error code */
            cout << "Opening of door @ location " << location
                 << " returned an error \"" <<  QCC_StatusText(status) << "\"" << endl;
        }
    }
}

static void close_door(BusAttachment& bus, Observer* observer, const string& location)
{
    DoorProxy door = get_door_at_location(bus, observer, location);

    if (door.IsValid()) {
        QStatus status = door.Close();

        if (ER_OK == status) {
            /* No error */
            cout << "Closing of door succeeded" << endl;
        } else if (ER_BUS_REPLY_IS_ERROR_MESSAGE == status) {
            /* MethodReply Error received (an error string) */
            cout << "Closing of door @ location " << location
                 << " returned an error." << endl;
        } else {
            /* Framework error or MethodReply error code */
            cout << "Closing of door @ location " << location
                 << " returned an error \"" <<  QCC_StatusText(status) << "\"" << endl;
        }
    }
}

static void knock_and_run(BusAttachment& bus, Observer* observer, const string& location)
{
    DoorProxy door = get_door_at_location(bus, observer, location);

    if (door.IsValid()) {
        if (ER_OK != door.KnockAndRun()) {
            cout << "A framework error occurred while trying to knock on door @ location "
                 << location << endl;
        }
    }
}

static bool parse(BusAttachment& bus, Observer* observer, const string& input)
{
    char cmd;
    size_t argpos;
    string arg = "";

    if (input.length() == 0) {
        return true;
    }

    cmd = input[0];
    argpos = input.find_first_not_of(" \t", 1);
    if (argpos != input.npos) {
        arg = input.substr(argpos);
    }

    switch (cmd) {
    case 'q':
        return false;

    case 'l':
        list_doors(bus, observer);
        break;

    case 'o':
        open_door(bus, observer, arg);
        break;

    case 'c':
        close_door(bus, observer, arg);
        break;

    case 'k':
        knock_and_run(bus, observer, arg);
        break;

    case 'h':
    default:
        help();
        break;
    }

    return true;
}

static QStatus BuildInterface(BusAttachment& bus)
{
    QStatus status;

    InterfaceDescription* intf = NULL;
    status = bus.CreateInterface(INTF_NAME, intf);
    assert(ER_OK == status);
    status = intf->AddProperty("IsOpen", "b", PROP_ACCESS_READ);
    assert(ER_OK == status);
    status = intf->AddPropertyAnnotation("IsOpen", "org.freedesktop.DBus.Property.EmitsChangedSignal", "true");
    assert(ER_OK == status);
    status = intf->AddProperty("Location", "s", PROP_ACCESS_READ);
    assert(ER_OK == status);
    status = intf->AddPropertyAnnotation("Location", "org.freedesktop.DBus.Property.EmitsChangedSignal", "true");
    assert(ER_OK == status);
    status = intf->AddProperty("KeyCode", "u", PROP_ACCESS_READ);
    assert(ER_OK == status);
    status = intf->AddPropertyAnnotation("KeyCode", "org.freedesktop.DBus.Property.EmitsChangedSignal", "invalidates");
    assert(ER_OK == status);

    status = intf->AddMethod("Open", "", "", "");
    assert(ER_OK == status);
    status = intf->AddMethod("Close", "", "", "");
    assert(ER_OK == status);
    status = intf->AddMethod("KnockAndRun", "", "", "", MEMBER_ANNOTATE_NO_REPLY);
    assert(ER_OK == status);

    status = intf->AddSignal("PersonPassedThrough", "s", "name");
    assert(ER_OK == status);

    intf->Activate();

    return status;
}

static QStatus SetupBusAttachment(BusAttachment& bus)
{
    QStatus status;
    status = bus.Start();
    assert(ER_OK == status);
    status = bus.Connect();
    assert(ER_OK == status);

    status = BuildInterface(bus);
    assert(ER_OK == status);

    return status;
}

class DoorListener :
    public MessageReceiver,
    public Observer::Listener,
    public ProxyBusObject::PropertiesChangedListener {
    static const char* props[];
  public:
    Observer* observer;
    BusAttachment* bus;

    virtual void ObjectDiscovered(ManagedProxyBusObject& proxy) {
        cout << "[listener] Door " << proxy->GetUniqueName() << ":"
             << proxy->GetPath() << " has just been discovered." << endl;

        bus->EnableConcurrentCallbacks();
        proxy->RegisterPropertiesChangedListener(INTF_NAME, props, 3, *this, NULL);

        DoorProxy door(proxy, *bus);
        string location;
        bool isopen;
        uint32_t keycode;
        if (ER_OK != door.GetAllProperties(isopen, location, keycode)) {
            cerr << "Could not retrieve door properties." << endl;
        } else {
            cout << "  location: " << location << endl;
            cout << "   is open: " << isopen << endl;
            cout << "   keycode: " << keycode << endl;
        }

        cout << "> ";
        cout.flush();
    }

    virtual void ObjectLost(ManagedProxyBusObject& proxy) {
        cout << "[listener] Door " << proxy->GetUniqueName() << ":"
             << proxy->GetPath() << " no longer exists." << endl;
        cout << "> ";
        cout.flush();
    }

    virtual void PropertiesChanged(ProxyBusObject& obj,
                                   const char* ifaceName,
                                   const MsgArg& changed,
                                   const MsgArg& invalidated,
                                   void* context) {
        QStatus status = ER_OK;
        bus->EnableConcurrentCallbacks();
        DoorProxy door(observer->Get(ObjectId(obj)), *bus);
        if (!door.IsValid()) {
            cerr << "Received a PropertiesChanged signal from a door we don't know." << endl;
            status = ER_FAIL;
        }
        string location;
        if (ER_OK == status) {
            status = door.GetLocation(location);
        }
        if (ER_OK == status) {
            cout << "Door @location " << location << " has updated state:" << endl;
        }

        size_t nelem;
        MsgArg* elems;
        if (ER_OK == status) {
            status = changed.Get("a{sv}", &nelem, &elems);
        }
        if (ER_OK == status) {
            for (size_t i = 0; i < nelem; ++i) {
                const char* prop;
                MsgArg* val;
                status = elems[i].Get("{sv}", &prop, &val);
                if (ER_OK == status) {
                    string propname = prop;
                    if (propname == "Location") {
                        const char* newloc;
                        status = val->Get("s", &newloc);
                        if (ER_OK == status) {
                            cout << "  location: " << newloc << endl;
                        }
                    } else if (propname == "IsOpen") {
                        bool isopen;
                        status = val->Get("b", &isopen);
                        if (ER_OK == status) {
                            cout << "   is open: " << isopen << endl;
                        }
                    }
                } else {
                    break;
                }
            }
        }

        if (ER_OK == status) {
            status = invalidated.Get("as", &nelem, &elems);
        }
        if (ER_OK == status) {
            for (size_t i = 0; i < nelem; ++i) {
                char* prop;
                status = elems[i].Get("s", &prop);
                if (status == ER_OK) {
                    cout << "  invalidated " << prop << endl;
                }
            }
        }

        cout << "> ";
        cout.flush();
    }

    void PersonPassedThrough(const InterfaceDescription::Member* member, const char* path, Message& message) {
        char* name;
        string location;
        QStatus status = message->GetArgs("s", &name);
        if (ER_OK == status) {
            bus->EnableConcurrentCallbacks();
            DoorProxy door(observer->Get(message->GetSender(), path), *bus);
            if (door.IsValid()) {
                status = door.GetLocation(location);
            } else {
                cerr << "Received a signal from a door we don't know." << endl;
                status = ER_FAIL;
            }
        }
        if (ER_OK == status) {
            cout << "[listener] " << name << " passed through a door "
                 << "@location " << location << endl;
            cout << "> ";
            cout.flush();
        }
    }
};

const char* DoorListener::props[] = {
    "IsOpen", "Location", "KeyCode"
};

int main(int argc, char** argv)
{
    if (AllJoynInit() != ER_OK) {
        return 1;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return 1;
    }
#endif

    BusAttachment* bus = new BusAttachment("door_consumer", true);
    SetupBusAttachment(*bus);

    const char* intfname = INTF_NAME;
    Observer* obs = new Observer(*bus, &intfname, 1);
    DoorListener* listener = new DoorListener();
    listener->observer = obs;
    listener->bus = bus;
    obs->RegisterListener(*listener);
    bus->RegisterSignalHandler(listener, static_cast<MessageReceiver::SignalHandler>(&DoorListener::PersonPassedThrough), bus->GetInterface(INTF_NAME)->GetMember("PersonPassedThrough"), NULL);

    bool done = false;
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !parse(*bus, obs, input);
    }

    // Cleanup
    obs->UnregisterAllListeners();
    bus->Stop();
    bus->Join();
    delete bus;
    delete obs;
    delete listener;

#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
    return 0;
}
