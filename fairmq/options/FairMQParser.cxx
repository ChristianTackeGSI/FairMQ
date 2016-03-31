/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/*
 * File:   FairMQParser.cxx
 * Author: winckler
 * 
 * Created on May 14, 2015, 5:01 PM
 */

#include "FairMQParser.h"
#include "FairMQLogger.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;

namespace FairMQParser
{

// TODO : add key-value map<string,string> parameter  for replacing/updating values from keys
// function that convert property tree (given the xml or json structure) to FairMQMap
FairMQMap ptreeToMQMap(const boost::property_tree::ptree& pt, const string& id, const string& rootNode, const string& formatFlag)
{
    // Create fair mq map
    FairMQMap channelMap;
    // helper::PrintDeviceList(pt.get_child(rootNode));
    // Extract value from boost::property_tree
    helper::DeviceParser(pt.get_child(rootNode), channelMap, id, formatFlag);
    if (channelMap.size() > 0)
    {
        stringstream channelKeys;
        for (const auto& p : channelMap)
        {
            channelKeys << "'" << p.first << "' ";
        }
        LOG(DEBUG) << "---- Found following channel keys: " << channelKeys.str();
    }
    else
    {
        LOG(WARN) << "---- No channel keys found for " << id;
        LOG(WARN) << "---- Check the JSON inputs and/or command line inputs";
    }

    return channelMap;
}

FairMQMap JSON::UserParser(const string& filename, const string& deviceId, const string& rootNode)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filename, pt);
    return ptreeToMQMap(pt, deviceId, rootNode);
}

FairMQMap JSON::UserParser(stringstream& input, const string& deviceId, const string& rootNode)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(input, pt);
    return ptreeToMQMap(pt, deviceId, rootNode);
}

FairMQMap XML::UserParser(const string& filename, const string& deviceId, const string& rootNode)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(filename, pt);
    return ptreeToMQMap(pt, deviceId, rootNode, "xml");
}

FairMQMap XML::UserParser(stringstream& input, const string& deviceId, const string& rootNode)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(input, pt);
    return ptreeToMQMap(pt, deviceId, rootNode, "xml");
}

namespace helper
{
    void PrintDeviceList(const boost::property_tree::ptree& tree, const std::string& formatFlag)
    {
        string deviceIdKey;

        // do a first loop just to print the device-id in json input 
        for (const auto& p : tree)
        {
            if (p.first == "devices")
            {
                for (const auto& q : p.second.get_child(""))
                {
                    string key = q.second.get<string>("key", "");
                    if (key != "")
                    {
                        deviceIdKey = key;
                        LOG(TRACE) << "Found config for device key '" << deviceIdKey << "' in JSON input";
                    }
                    else
                    {
                        deviceIdKey = q.second.get<string>("id");
                        LOG(TRACE) << "Found config for device id '" << deviceIdKey << "' in JSON input";
                    }
                }
            }

            if (p.first == "device")
            {
                //get id attribute to choose the device
                if (formatFlag == "xml")
                {
                    deviceIdKey = p.second.get<string>("<xmlattr>.id");
                    LOG(TRACE) << "Found config for '" << deviceIdKey << "' in XML input";
                }

                if (formatFlag == "json")
                {
                    string key = p.second.get<string>("key", "");
                    if (key != "")
                    {
                        deviceIdKey = key;
                        LOG(TRACE) << "Found config for device key '" << deviceIdKey << "' in JSON input";
                    }
                    else
                    {
                        deviceIdKey = p.second.get<string>("id");
                        LOG(TRACE) << "Found config for device id '" << deviceIdKey << "' in JSON input";
                    }
                }
            }
        }
    }

    void DeviceParser(const boost::property_tree::ptree& tree, FairMQMap& channelMap, const string& deviceId, const string& formatFlag)
    {
        string deviceIdKey;

        LOG(DEBUG) << "Looking for '" << deviceId << "' id/key in the provided config file...";

        // For each node in fairMQOptions
        for (const auto& p0 : tree)
        {
            if (p0.first == "devices")
            {
                for (const auto& p : p0.second)
                {
                    // check if key is provided, otherwise use id
                    string key = p.second.get<string>("key", "");
                    if (key != "")
                    {
                        deviceIdKey = key;
                        // LOG(DEBUG) << "Found config for device key '" << deviceIdKey << "' in JSON input";
                    }
                    else
                    {
                        deviceIdKey = p.second.get<string>("id");
                        // LOG(DEBUG) << "Found config for device id '" << deviceIdKey << "' in JSON input";
                    }

                    // if not correct device id, do not fill MQMap
                    if (deviceId != deviceIdKey)
                    {
                        continue;
                    }

                    LOG(DEBUG) << "[" << p0.first << "] " << deviceIdKey;
                    ChannelParser(p.second, channelMap, formatFlag);
                }
            }

            if (p0.first == "device")
            {
                if (formatFlag == "xml")
                {
                    deviceIdKey = p0.second.get<string>("<xmlattr>.id");
                    LOG(DEBUG) << "Found config for '" << deviceIdKey << "' in XML input";
                }

                if (formatFlag == "json")
                {
                    // check if key is provided, otherwise use id
                    string key = p0.second.get<string>("key", "");
                    if (key != "")
                    {
                        deviceIdKey = key;
                        // LOG(DEBUG) << "Found config for device key '" << deviceIdKey << "' in JSON input";
                    }
                    else
                    {
                        deviceIdKey = p0.second.get<string>("id");
                        // LOG(DEBUG) << "Found config for device id '" << deviceIdKey << "' in JSON input";
                    }
                }

                // if not correct device id, do not fill MQMap
                if (deviceId != deviceIdKey)
                {
                    continue;
                }

                LOG(DEBUG) << "[" << p0.first << "] " << deviceIdKey;

                ChannelParser(p0.second, channelMap, formatFlag);
            }
        }
    }

    void ChannelParser(const boost::property_tree::ptree& tree, FairMQMap& channelMap, const string& formatFlag)
    {
        string channelKey;

        for (const auto& p : tree)
        {
            if (p.first == "channels")
            {
                for (const auto& q : p.second)
                {
                    channelKey = q.second.get<string>("name");

                    // try to get common properties to use for all subChannels
                    FairMQChannel commonChannel;
                    commonChannel.UpdateType(q.second.get<string>("type", commonChannel.GetType()));
                    commonChannel.UpdateMethod(q.second.get<string>("method", commonChannel.GetMethod()));
                    commonChannel.UpdateProperty(q.second.get<string>("property", commonChannel.GetProperty()));
                    commonChannel.UpdateSndBufSize(q.second.get<int>("sndBufSize", commonChannel.GetSndBufSize()));
                    commonChannel.UpdateRcvBufSize(q.second.get<int>("rcvBufSize", commonChannel.GetRcvBufSize()));
                    commonChannel.UpdateRateLogging(q.second.get<int>("rateLogging", commonChannel.GetRateLogging()));

                    LOG(DEBUG) << "\t[" << p.first << "] " << channelKey;

                    // temporary FairMQChannel container
                    vector<FairMQChannel> channelList;
                    SocketParser(q.second.get_child(""), channelList, commonChannel);

                    channelMap.insert(make_pair(channelKey, move(channelList)));
                }
            }

            if (p.first == "channel")
            {
                // try to get common properties to use for all subChannels
                FairMQChannel commonChannel;

                // get name attribute to form key
                if (formatFlag == "xml")
                {
                    channelKey = p.second.get<string>("<xmlattr>.name");
                }

                if (formatFlag == "json")
                {
                    channelKey = p.second.get<string>("name");

                    // try to get common properties to use for all subChannels
                    commonChannel.UpdateType(p.second.get<string>("type", commonChannel.GetType()));
                    commonChannel.UpdateMethod(p.second.get<string>("method", commonChannel.GetMethod()));
                    commonChannel.UpdateProperty(p.second.get<string>("property", commonChannel.GetProperty()));
                    commonChannel.UpdateSndBufSize(p.second.get<int>("sndBufSize", commonChannel.GetSndBufSize()));
                    commonChannel.UpdateRcvBufSize(p.second.get<int>("rcvBufSize", commonChannel.GetRcvBufSize()));
                    commonChannel.UpdateRateLogging(p.second.get<int>("rateLogging", commonChannel.GetRateLogging()));
                }

                LOG(DEBUG) << "\t[" << p.first  << "] " << channelKey;

                // temporary FairMQChannel container
                vector<FairMQChannel> channelList;
                SocketParser(p.second.get_child(""), channelList, commonChannel);

                channelMap.insert(make_pair(channelKey, move(channelList)));
            }
        }
    }

    void SocketParser(const boost::property_tree::ptree& tree, vector<FairMQChannel>& channelList, const FairMQChannel& commonChannel)
    {
        // for each socket in channel
        int socketCounter = 0;
        for (const auto& p : tree)
        {
            if (p.first == "sockets")
            {
                for (const auto& q : p.second)
                {
                    ++socketCounter;
                    // create new channel and apply setting from the common channel
                    FairMQChannel channel(commonChannel);

                    // if the socket field specifies or overrides something from the common channel, apply those settings
                    channel.UpdateType(q.second.get<string>("type", channel.GetType()));
                    channel.UpdateMethod(q.second.get<string>("method", channel.GetMethod()));
                    channel.UpdateAddress(q.second.get<string>("address", channel.GetAddress()));
                    channel.UpdateProperty(q.second.get<string>("property", channel.GetProperty()));
                    channel.UpdateSndBufSize(q.second.get<int>("sndBufSize", channel.GetSndBufSize()));
                    channel.UpdateRcvBufSize(q.second.get<int>("rcvBufSize", channel.GetRcvBufSize()));
                    channel.UpdateRateLogging(q.second.get<int>("rateLogging", channel.GetRateLogging()));

                    LOG(DEBUG) << "\t\t[" << p.first  << "] " << socketCounter;
                    LOG(DEBUG) << "\t\t\ttype        = " << channel.GetType();
                    LOG(DEBUG) << "\t\t\tmethod      = " << channel.GetMethod();
                    LOG(DEBUG) << "\t\t\taddress     = " << channel.GetAddress();
                    LOG(DEBUG) << "\t\t\tproperty    = " << channel.GetProperty();
                    LOG(DEBUG) << "\t\t\tsndBufSize  = " << channel.GetSndBufSize();
                    LOG(DEBUG) << "\t\t\trcvBufSize  = " << channel.GetRcvBufSize();
                    LOG(DEBUG) << "\t\t\trateLogging = " << channel.GetRateLogging();

                    channelList.push_back(channel);
                }
            }

            if (p.first == "socket")
            {
                ++socketCounter;
                // create new channel and apply setting from the common channel
                FairMQChannel channel(commonChannel);

                // if the socket field specifies or overrides something from the common channel, apply those settings
                channel.UpdateType(p.second.get<string>("type", channel.GetType()));
                channel.UpdateMethod(p.second.get<string>("method", channel.GetMethod()));
                channel.UpdateAddress(p.second.get<string>("address", channel.GetAddress()));
                channel.UpdateProperty(p.second.get<string>("property", channel.GetProperty()));
                channel.UpdateSndBufSize(p.second.get<int>("sndBufSize", channel.GetSndBufSize()));
                channel.UpdateRcvBufSize(p.second.get<int>("rcvBufSize", channel.GetRcvBufSize()));
                channel.UpdateRateLogging(p.second.get<int>("rateLogging", channel.GetRateLogging()));

                LOG(DEBUG) << "\t\t[" << p.first  << "] " << socketCounter;
                LOG(DEBUG) << "\t\t\ttype        = " << channel.GetType();
                LOG(DEBUG) << "\t\t\tmethod      = " << channel.GetMethod();
                LOG(DEBUG) << "\t\t\taddress     = " << channel.GetAddress();
                LOG(DEBUG) << "\t\t\tproperty    = " << channel.GetProperty();
                LOG(DEBUG) << "\t\t\tsndBufSize  = " << channel.GetSndBufSize();
                LOG(DEBUG) << "\t\t\trcvBufSize  = " << channel.GetRcvBufSize();
                LOG(DEBUG) << "\t\t\trateLogging = " << channel.GetRateLogging();

                channelList.push_back(channel);
            }
        } // end socket loop

        if (socketCounter)
        {
            LOG(DEBUG) << "Found " << socketCounter << " socket(s) in channel.";
        }
        else
        {
            LOG(DEBUG) << "\t\t\tNo subChannels specified,";
            LOG(DEBUG) << "\t\t\tapplying common settings to the channel:";
            FairMQChannel channel(commonChannel);

            LOG(DEBUG) << "\t\t\ttype        = " << channel.GetType();
            LOG(DEBUG) << "\t\t\tmethod      = " << channel.GetMethod();
            LOG(DEBUG) << "\t\t\taddress     = " << channel.GetAddress();
            LOG(DEBUG) << "\t\t\tproperty    = " << channel.GetProperty();
            LOG(DEBUG) << "\t\t\tsndBufSize  = " << channel.GetSndBufSize();
            LOG(DEBUG) << "\t\t\trcvBufSize  = " << channel.GetRcvBufSize();
            LOG(DEBUG) << "\t\t\trateLogging = " << channel.GetRateLogging();

            channelList.push_back(channel);
        }

    }
} // end helper namespace

} //  end FairMQParser namespace
