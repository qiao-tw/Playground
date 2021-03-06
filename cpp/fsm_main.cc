#include "fsm.h"
#include "datatype/ids.h"
#include "datatype/types.h"
#include "msgbus.h"
#include <iostream>

using namespace std;

using ThreadPtr = shared_ptr<thread>;


class TestNode : public MsgBus::Subscriber {
public:
  enum class TestEventId
  {
      A
  };

  enum class TestStateId
  {
      IDLE,
      START
  };

  using TestStateMachine = EvStateMachineBase<TestStateId, TestEventId>;  

  struct EventA : public TestStateMachine::EventBase {
    int data;
    EventA(int d) : EventBase(TestEventId::A, "EV_A"), data(d) {}
  };

  class IdleState : public TestStateMachine::EvStateBase
  {
  public:
    IdleState() : EvStateBase(TestStateId::IDLE, "IDLE") {}
    void OnEvent(shared_ptr<TestStateMachine::EventBase> ev) override {
      switch (ev->ev_id) {
      case TestEventId::A: {
        cout << ev->ev_name << endl;
        shared_ptr<EventA> event_a = dynamic_pointer_cast<EventA>(ev);
        cout << "data: " << event_a->data << endl;
        TransitTo(make_shared<StartState>(event_a->data));
      } break;
      default:
        cout << "IdleState ignores event: " << ev->ev_name << endl;
      }
    }
    void ActionEntry() override 
    { 
        cout << "entering IdleState" << endl; 
    }
    void ActionExit() override { cout << "exiting IdleState" << endl; }
  };

  class StartState : public TestStateMachine::EvStateBase
  {
  public:
    StartState(int d) : EvStateBase(TestStateId::START, "START"), data(d) {}
    void OnEvent(std::shared_ptr<TestStateMachine::EventBase> ev) override {
      switch (ev->ev_id) {
      case TestEventId::A: {
        cout << ev->ev_name << endl;
        shared_ptr<EventA> event_a = dynamic_pointer_cast<EventA>(ev);
        cout << "data: " << event_a->data << endl;
        TransitTo(make_shared<IdleState>());
      } break;
      default:
        cout << "StartState ignores event: " << ev->ev_name << endl;
      }
    }
    void ActionEntry() override {
      cout << "entering StartState: data(" << data << ")" << endl;
    }
    void ActionExit() override { cout << "exiting StartState" << endl; }

  private:
    int data;
  };

  TestNode(): MsgBus::Subscriber("test_node") {
    state_machine_ = make_shared<TestStateMachine>("test_node");
  }

  virtual ~TestNode() = default;

  void Activate() 
  {
    state_machine_->Start(make_shared<IdleState>());
  }

  void Deactivate()
  {
    state_machine_->Stop();
  }

  // MsgHandler::ProcMsg
  bool ProcMsg(MsgPtr msg) override {
    switch (msg->data_id) {
    case DataType::SYS_INFO: {
      shared_ptr<SysInfoMsg> msg_a = dynamic_pointer_cast<SysInfoMsg>(msg);
      cout << msg_a->data.boot_cnt << endl;
      state_machine_->EmitEvent(make_shared<EventA>(msg_a->data.boot_cnt));
    } 
    break;
    default:
      cout << "ignore msg_type: " << msg->msg_name;
    }
    return true;
  }

private:
  shared_ptr<TestStateMachine> state_machine_;  

};

int main(int argc, char *argv[]) {
  shared_ptr<TestNode> tn = make_shared<TestNode>();
  // tn->Activate();

  int k = 0;
  while (true) {
    tn->Activate();
    for (int i = 0; i < 10; i++) {
      tn->Dispatch(make_shared<SysInfoMsg>(SysInfo(true, k++)));
      this_thread::sleep_for(chrono::milliseconds(100));
    }
    tn->Deactivate();
    this_thread::sleep_for(chrono::seconds(1));
  }

  return 0;
}