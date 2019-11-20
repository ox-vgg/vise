/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#ifndef _SLOW_CONSTRUCTION_H_
#define _SLOW_CONSTRUCTION_H_

#include <utility>

#include <boost/function.hpp>
#include <boost/thread.hpp>



// Typically slowConstruction is useful for loading indexes from disk.
// If we load them all at the same time, it will be slower. So this is the class
// which enables this behaviour.
// Careful:
// 1) Don't delete this object before the end of the program as it will likely
//    delete the created objects.
// 2) Delete this before the objects which used it at construction time, as otherwise
//    the deletion procedure is messed up (this tries to call cleanup functions of
//    already deleted stuff).
class sequentialConstructions {
    
    public:
        
        sequentialConstructions(): started(false) {}
        ~sequentialConstructions();
        
        void addFunction(boost::function<void()> f);
        void addCleanup(boost::function<void()> f);
        
        void start();
    
    private:
        void reallyStart();
        
        bool started;
        std::vector< boost::function<void()> > fs_;
        std::vector< boost::function<void()> > cleanups_;
        boost::thread *t_;
};




// Given an object A and a constructor of an object B, this class
// starts the construction of object B. The provided
// getObject() method returns immediately object B if it has been constructed,
// or object A otherwise. It is useful for, for example, inverted index
// where A= index being read from disk (i.e. slow retrieval) and
// B= index loaded into ram (i.e. fast retreival but slow construction)

template <class T>
class slowConstruction {
    
    public:
        
        class helper;
        
        slowConstruction( T *first, boost::function<T*()> secondConstructor, bool deleteFirst= false, sequentialConstructions* consQueue= NULL ) :
            finished_(false), first_(first), object_(first) {
            boost::function<void()> constructAndSwitch_= boost::bind(
                &slowConstruction::constructAndSwitch, this, secondConstructor, deleteFirst);
            if (consQueue==NULL)
                constructor_= new boost::thread(constructAndSwitch_);
            else {
                constructor_= NULL;
                consQueue->addFunction(constructAndSwitch_);
                consQueue->addCleanup(boost::bind(&slowConstruction::cleanup, this));
            }
        }
        
        virtual ~slowConstruction() {
            if (constructor_!=NULL) {
                // wait until the constructor finishes in order to delete its memory and second_
                constructor_->join(); // this itself deletes first if requested
                delete constructor_;
                cleanup(); // if sequentialConstructions is used, it performs the cleanup
            }
        }
        
        void cleanup(){
            delete second_;
        }
        
        typedef boost::shared_lock<boost::shared_mutex> readLock;
        
        // Careful how this is used:
        // 1) Don't delete the object (from helper)
        // 2) Do not store the pointer to the returned object
        //    if finished()==false as the pointer will change (and potentially get deleted!)
        //    after object B is constructed
        // 3) if finished()==false: delete the returned helper as soon as possible
        helper getObject() const {
            if (finished_)
                return helper(object_, NULL);
            // construction still in progress, need to check if switch is happening right now
            readLock* readLock_= new readLock(objectLock_);
            return helper(object_, readLock_);
        }
        
        // Careful: the state of this can change from false to true (never true->false)
        bool finished() const {
            return finished_;
        }
        
        class helper {
            public:
                helper(T* object,readLock* lock) : object_(object), lock_(lock) {}
                ~helper(){ if (lock_!=NULL) delete lock_; }
                T* operator->(){ return object_; }
            private:
                T* object_;
                readLock* lock_;
        };
    
    private:
        
        void constructAndSwitch(boost::function<T*()> secondConstructor, bool deleteFirst){
            second_= secondConstructor();
            {
                boost::upgrade_lock< boost::shared_mutex > lock(objectLock_);
                boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
                object_= second_;
                finished_= true;
            }
            if (deleteFirst)
                delete first_;
        }
        
        bool finished_;
        T *first_, *second_, *object_;
        boost::thread *constructor_;
        boost::shared_mutex mutable objectLock_;
};



#endif
