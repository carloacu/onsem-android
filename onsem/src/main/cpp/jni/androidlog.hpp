#ifndef ROBOTPLANNER_ANDROID_ANDROIDLOGS_HPP
#define ROBOTPLANNER_ANDROID_ANDROIDLOGS_HPP

#ifdef COUT_TO_ANDROID_LOG
# include <android/log.h>
# include <iostream>
#endif // COUT_TO_ANDROID_LOG

namespace {
#ifdef COUT_TO_ANDROID_LOG
    // https://stackoverflow.com/questions/8870174/is-stdcout-usable-in-android-ndk
    class forward_to_android : public std::streambuf {
    public:
        enum {
            bufsize = 1024
        }; // ... or some other suitable buffer size
        forward_to_android() { this->setp(buffer, buffer + bufsize - 1); }

    private:
        int overflow(int c) override {
            if (c == traits_type::eof()) {
                *this->pptr() = traits_type::to_char_type(c);
                this->sbumpc();
            }
            return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
        }

        int sync() override {
            int rc = 0;
            if (this->pbase() != this->pptr()) {
                char writebuf[bufsize + 1];
                memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
                writebuf[this->pptr() - this->pbase()] = '\0';

                rc = __android_log_write(ANDROID_LOG_DEBUG, "native", writebuf) > 0;
                this->setp(buffer, buffer + bufsize - 1);
            }
            return rc;
        }

        char buffer[bufsize];
    };
#endif // COUT_TO_ANDROID_LOG
}


#endif // ROBOTPLANNER_ANDROID_ANDROIDLOGS_HPP
