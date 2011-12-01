/** Stores and provides access to the paths between stations.
 *
 * Paths are hard coded in station_path.cc
 */

#ifndef STATION_PATH_H_
#define STATION_PATH_H_

#include <vector>
#include <exception>
#include <string>


class StationList;

/// A Class to represent a station (name and code)
class Station
{
private:
    friend class StationList;
    std::string name_;
    unsigned number_;
    bool valid_;
    Station(const std::string & name, unsigned number) : name_(name), number_(number), valid_(true) { }

public:
    Station() : valid_(false) { }
    unsigned number() const throw() { return number_; }
    const std::string & str() const throw() { return name_; }
    const char * c_str() const throw() { return name_.c_str(); }
    bool operator== (const Station & s) const { return s.valid_ && valid_ && s.number_==number_; }
    bool operator!= (const Station & s) const { return !(s==*this); }
};


class StationDoesNotExistException : public std::exception
{
    std::string msg;
public:
    StationDoesNotExistException() throw() : std::exception() { }
    StationDoesNotExistException(const std::string & s) throw() : std::exception(), msg(s) { }
    StationDoesNotExistException(unsigned i) throw() : std::exception(), msg(""+i) { }
    ~StationDoesNotExistException() throw() { }
    const char* what() const throw() { return msg.c_str(); }
};


/// A class to hold the list of known stations, and convenience operators to
/// convert between station name and code.
class StationList
{
private:
    std::vector<Station> knownStations_;

public:
    StationList();

    /// Returns a station from its code
    const Station & operator () (unsigned i) const throw(StationDoesNotExistException);

    /// Returns the code of a station from its name
    const Station & operator() (const std::string & name) const throw(StationDoesNotExistException);

    /// Returns a station from its code
    const Station & get(unsigned i) const throw(StationDoesNotExistException);

    /// Returns the code of a station from its name
    const Station & get(const std::string & name) const throw(StationDoesNotExistException);

    /// Checks whether a station exists
    bool exists(const Station &) const throw();

    /// Checks whether a station code exists
    bool exists(unsigned) const throw();

    /// Checks whether a station name exists
    bool exists(const std::string &) const throw();

    /// Returns the number of known stations
    unsigned size() const { return knownStations_.size(); }

    /// Returns the list of known stations
    const std::vector<Station> & stations() const { return knownStations_; }

    typedef std::vector<Station>::const_iterator StationIterator;
    StationIterator begin() const { return knownStations_.begin(); }
    StationIterator end() const { return knownStations_.end(); }

    void print() const;
    const Station & prompt(const std::string & prompt) const throw(StationDoesNotExistException);
};


class PathPoint
{
public:
    double x_, y_;

    PathPoint() : x_(0), y_(0) { }
    PathPoint(double x, double y) : x_(x), y_(y) { }

    static double distance(const PathPoint &p1, const PathPoint &p2);
};

class StationPath : public std::vector<PathPoint>
{
    double length_;
public:
    StationPath() : length_(0.0) { }
    double recomputeLength();
    double length() const { return length_; }
};


/// A class that holds the paths between the different stations.
class StationPaths
{
public:
    StationPaths();

    /// Returns the list of known stations
    const StationList & knownStations() const { return knownStations_; }

    /// Returns the path between 2 stations
    const StationPath & getPath(const Station & pickup, const Station & dropoff) const;

private:
    std::vector< std::vector<StationPath> > stationPaths_;
    StationList knownStations_; ///< The list of know stations
};


#endif /* STATION_PATH_H_ */
