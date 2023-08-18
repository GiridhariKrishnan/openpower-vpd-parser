#pragma once

#include "types.hpp"

#include <errno.h>

#include <array>
#include <iostream>
#include <source_location>
#include <span>
#include <string_view>
#include <vector>

namespace vpd
{
/**
 * @brief The namespace defines utlity methods required for processing of VPD.
 */
namespace utils
{

/**
 * @brief An API to get Map of service and interfaces for an object path.
 *
 * The API returns a Map of service name and interfaces for a given pair of
 * object path and interface list. It can be used to determine service name
 * which implemets a particular object path and interface.
 *
 * Note: It will be caller's responsibility to check for empty map returned and
 * generate appropriate error.
 *
 * @param [in] objectPath - Object path under the service.
 * @param [in] interfaces - Array of interface(s).
 * @return - A Map of service name to object to interface(s), if success.
 *           If failed,  empty map.
 */
types::MapperGetObject getObjectMap(const std::string& objectPath,
                                    std::span<const char*> interfaces);

/**
 * @brief An API to read property from Dbus.
 *
 * The caller of the API needs to validate the validatity and correctness of the
 * type and value of data returned. The API will just fetch and retun the data
 * without any data validation.
 *
 * Note: It will be caller's responsibility to check for empty value returned
 * and generate appropriate error if required.
 *
 * @param [in] serviceName - Name of the Dbus service.
 * @param [in] objectPath - Object path under the service.
 * @param [in] interface - Interface under which property exist.
 * @param [in] property - Property whose value is to be read.
 * @return - Value read from Dbus, if success.
 *           If failed, empty variant.
 */
types::DbusVariantType readDbusProperty(const std::string& serviceName,
                                        const std::string& objectPath,
                                        const std::string& interface,
                                        const std::string& property);

/**
 * @brief An API to write property on Dbus.
 *
 * The caller of this API needs to handle exception thrown by this method to
 * identify any write failure. The API in no other way indicate write  failure
 * to the caller.
 *
 * Note: It will be caller's responsibility ho handle the exception thrown in
 * case of write failure and generate appropriate error.
 *
 * @param [in] serviceName - Name of the Dbus service.
 * @param [in] objectPath - Object path under the service.
 * @param [in] interface - Interface under which property exist.
 * @param [in] property - Property whose value is to be written.
 * @param [in] propertyValue - The value to be written.
 */
void writeDbusProperty(const std::string& serviceName,
                       const std::string& objectPath,
                       const std::string& interface,
                       const std::string& property,
                       const types::DbusVariantType& propertyValue);

/**
 * @brief API to generate file name for bad VPD.
 *
 * For i2c eeproms - the pattern of the vpd-name will be
 * i2c-<bus-number>-<eeprom-address>.
 * For spi eeproms - the pattern of the vpd-name will be spi-<spi-number>.
 *
 * @param[in] vpdFilePath - file path of the vpd.
 * @return Generated file name.
 */
std::string generateBadVPDFileName(const std::string& vpdFilePath);

/**
 * @brief API which dumps the broken/bad vpd in a directory.
 * When the vpd is bad, this api places  the bad vpd file inside
 * "/tmp/bad-vpd" in BMC, in order to collect bad VPD data as a part of user
 * initiated BMC dump.
 *
 * Note: Throws exception in case of any failure.
 *
 * @param[in] vpdFilePath - vpd file path
 * @param[in] vpdVector - vpd vector
 */
void dumpBadVpd(const std::string& vpdFilePath,
                const types::BinaryVector& vpdVector);

/**
 * @brief API to return null at the end of variadic template args.
 *
 * @return empty string.
 */
inline std::string getCommand()
{
    return "";
}

/**
 * @brief API to arrange create command.
 *
 * @param[in] arguments to create the command
 * @return cmd - command string
 */
template <typename T, typename... Types>
inline std::string getCommand(T arg1, Types... args)
{
    std::string cmd = " " + arg1 + getCommand(args...);

    return cmd;
}

/**
 * @brief API to create shell command and execute.
 *
 * Note: Throws exception on any failure. Caller needs to handle.
 *
 * @param[in] arguments for command
 * @returns output of that command
 */
template <typename T, typename... Types>
inline std::vector<std::string> executeCmd(T&& path, Types... args)
{
    std::vector<std::string> cmdOutput;
    std::array<char, 128> buffer;

    std::string cmd = path + getCommand(args...);

    std::unique_ptr<FILE, decltype(&pclose)> cmdPipe(popen(cmd.c_str(), "r"),
                                                     pclose);
    if (!cmdPipe)
    {
        std::cerr << "popen failed with error" << strerror(errno) << std::endl;
        throw std::runtime_error("popen failed!");
    }
    while (fgets(buffer.data(), buffer.size(), cmdPipe.get()) != nullptr)
    {
        cmdOutput.emplace_back(buffer.data());
    }

    return cmdOutput;
}

/**
 * @brief An api to read value of a keyword.
 *
 * Note: Throws exception. Caller needs to handle.
 *
 * @param[in] kwdValueMap - A map having Kwd value pair.
 * @param[in] kwd - keyword name.
 * @param[out] kwdValue - Value of the keyword read from map.
 */
void getKwVal(const types::KwdValueMap& kwdValueMap, const std::string& kwd,
              std::string& kwdValue);

/**
 * @brief API to publish data on PIM
 *
 * The API calls notify on PIM object to publlish VPD.
 *
 * @param[in] objectMap - Object, its interface and data.
 * @return bool - Status of call to PIM notify.
 */
bool callPIM(types::ObjectMap&& objectMap);

} // namespace utils
} // namespace vpd