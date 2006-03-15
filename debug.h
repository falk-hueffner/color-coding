#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

class logstream {
public:
    logstream(std::ostream& n_out, bool n_on = false) : out(n_out), on(n_on) { }
    template<typename T>
    logstream& operator<<(const T& value) {
	if (on)
	    out << value;
	return *this;
    }
    logstream& operator<<(std::ostream& (*manip)(std::ostream&)) {
	if (on)
	    (*manip)(out);
	return *this;
    }

    void turnOn() { on = true; }

private:
    std::ostream& out;
    bool on;
};

extern logstream info, debug;

#endif // DEBUG_H
