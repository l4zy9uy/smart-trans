// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <string>

namespace logging   = boost::log;
namespace keywords  = boost::log::keywords;
namespace expr      = boost::log::expressions;
namespace attrs     = boost::log::attributes;

class Logger {
public:
    explicit Logger(const std::string& filename)
      : filename_(filename)
    {
        // 1) write CSV header (truncate if exists)
        std::ofstream hdr(filename_, std::ios::trunc);
        hdr << "EventType,VehicleID,TravelTime,TimeDeviation,ObjectiveValue,"
               "MapName,TotalVehicles,AlgorithmName,Version,VehiclesNotArrived,"
               "SumObjectiveValue,SumTravelTime,SumTimeDeviation,SumAlgorithmTime,SimulationStartTime\n";
        hdr.close();

        // 2) configure Boost.Log to append raw messages (our CSV lines) to the same file
        logging::add_file_log(
            keywords::file_name    = filename_,
            keywords::open_mode    = std::ios_base::app,
            keywords::auto_flush   = true,
            keywords::format       = expr::stream << expr::message
        );
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
        logging::add_common_attributes();

        // record simulation start time
        simulationStartTime_ = boost::posix_time::microsec_clock::local_time();
    }

    // call this whenever a vehicle i reaches its destination
    void logVehicleArrival(int vehicleId,
                           double travelTime,
                           double timeDeviation,
                           double objectiveValue)
    {
        // ARRIVAL: fill first 5 columns, then leave the next 10 blank
        BOOST_LOG_TRIVIAL(info)
            << "ARRIVAL,"
            << vehicleId << ","
            << travelTime << ","
            << timeDeviation << ","
            << objectiveValue
            // 10 commas → 10 blank fields for MapName…SimulationStartTime
            << ",,,,,,,,,,";
    }

    // call this once at the very end of the run
    void logSummary(const std::string& mapName,
                    int totalVehicles,
                    const std::string& algorithmName,
                    const std::string& version,
                    int vehiclesNotArrived,
                    double sumObjectiveValue,
                    double sumTravelTime,
                    double sumTimeDeviation,
                    double sumAlgorithmTime)
    {
        // build a simple string timestamp
        std::string startTimeStr = boost::posix_time::to_simple_string(simulationStartTime_);

        // SUMMARY: leave VehicleID…ObjectiveValue blank, then fill the summary fields
        BOOST_LOG_TRIVIAL(info)
            << "SUMMARY,,,,"           // 4 commas for 4 blank fields after EventType
            << mapName   << ","
            << totalVehicles  << ","
            << algorithmName  << ","
            << version        << ","
            << vehiclesNotArrived   << ","
            << sumObjectiveValue    << ","
            << sumTravelTime        << ","
            << sumTimeDeviation     << ","
            << sumAlgorithmTime     << ","
            << startTimeStr;
    }

private:
    std::string filename_;
    boost::posix_time::ptime simulationStartTime_;
};

#endif // LOGGER_H
