#ifndef _paintown_joystick_h
#define _paintown_joystick_h

#include <vector>
#include <set>

struct JoystickInput{
    JoystickInput():
        left(false),
        right(false),
        up(false),
        down(false),
        button1(false),
        button2(false),
        button3(false),
        button4(false),
        button5(false),
        button6(false),
        quit(false),
        start(false){
    }

    JoystickInput(const JoystickInput & copy):
        left(copy.left),
        right(copy.right),
        up(copy.up),
        down(copy.down),
        button1(copy.button1),
        button2(copy.button2),
        button3(copy.button3),
        button4(copy.button4),
        button5(copy.button5),
        button6(copy.button6),
        quit(copy.quit),
        start(copy.start){
    }

    /* true if something is pressed */
    bool pressed(){
        return left || right || up || down ||
               button1 || button2 || button3 || button4 ||
               button5 || button6 ||
               quit || start;
    }

    bool left;
    bool right;
    bool up;
    bool down;
    bool button1;
    bool button2;
    bool button3;
    bool button4;
    bool button5;
    bool button6;
    bool quit;
    bool start;
};

/* Callback for joystick actions like button presses, axis motions, etc.
 */
class Joystick;
class JoystickListener{
public:
    JoystickListener();
    virtual ~JoystickListener();
    virtual void pressButton(Joystick * from, int button) = 0;
    virtual void releaseButton(Joystick * from, int button) = 0;
    virtual void axisMotion(Joystick * from, int axis, int motion) = 0;
    virtual void hatMotion(Joystick * from, int motion) = 0;
};

class Joystick{
public:
    virtual void poll() = 0;
    // virtual JoystickInput readAll() = 0;
    virtual bool pressed() const;
    virtual ~Joystick();

    virtual int getDeviceId() const = 0;
    virtual void pressButton(int button);
    virtual void releaseButton(int button);
    virtual void axisMotion(int axis, int motion);
    virtual void hatMotion(int motion);

    /* create the ith joystick */
    static Joystick * create(int i);
    static int numberOfJoysticks();

    enum Key{
        Invalid = -1,
        Up = 0,
        Down,
        Left,
        Right,
        Button1,
        Button2,
        Button3,
        Button4,
        Button5,
        Button6,
        Quit,
        Start
    };

    struct Event{
        Event(Key key, bool enabled):
        key(key), enabled(enabled){
        }

        Key key;
        bool enabled;
    };
    
    virtual inline const std::vector<Event> & getEvents() const {
        return events;
    }
	
    static const char * keyToName(Key key);

    static void addListener(JoystickListener * listener);
    static void removeListener(JoystickListener * listener);

protected:

    static std::set<JoystickListener*> getListeners();

    static std::set<JoystickListener*> listeners;
    std::vector<Event> events;

    Joystick();
};

#endif
