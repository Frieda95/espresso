#ifndef ESPRESSO_SCRIPT_INTERFACE_OBJECTMANAGER_HPP
#define ESPRESSO_SCRIPT_INTERFACE_OBJECTMANAGER_HPP

#include "Context.hpp"
#include "MpiCallbacks.hpp"
#include "ObjectHandle.hpp"
#include "PackedVariant.hpp"

#include <utils/Factory.hpp>

#include <boost/serialization/utility.hpp>

namespace ScriptInterface {

class GlobalContext : public Context {
  using ObjectId = std::size_t;

private:
  /* Instances on this node that are managed by the
   * head node. */
  std::unordered_map<ObjectId, ObjectRef> m_local_objects;

  Utils::Factory<ObjectHandle> m_factory;

public:
  auto &factory() { return m_factory; }

  auto const &local_objects() const { return m_local_objects; }

  template <class T> void register_new(const char *name) {
    m_factory.register_new<T>(name);
  }

private:
  Communication::CallbackHandle<ObjectId, const std::string &,
                                const PackedMap &>
      cb_make_handle;
  Communication::CallbackHandle<ObjectId, const std::string &,
                                const PackedVariant &>
      cb_set_parameter;
  Communication::CallbackHandle<ObjectId, std::string const &,
                                PackedMap const &>
      cb_call_method;
  Communication::CallbackHandle<ObjectId> cb_delete_handle;

public:
  GlobalContext(Communication::MpiCallbacks &callbacks,
                Utils::Factory<ObjectHandle> factory)
      : m_factory(std::move(factory)),
        cb_make_handle(&callbacks,
                       [this](ObjectId id, const std::string &name,
                              const PackedMap &parameters) {
                         make_handle(id, name, parameters);
                       }),
        cb_set_parameter(&callbacks,
                         [this](ObjectId id, std::string const &name,
                                PackedVariant const &value) {
                           set_parameter(id, name, value);
                         }),
        cb_call_method(&callbacks,
                       [this](ObjectId id, std::string const &name,
                              PackedMap const &arguments) {
                         call_method(id, name, arguments);
                       }),
        cb_delete_handle(&callbacks,
                         [this](ObjectId id) { delete_handle(id); }) {}

private:
  /**
   * @brief Callback for @function remote_make_handle
   */
  void make_handle(ObjectId id, const std::string &name,
                   const PackedMap &parameters);

public:
  /**
   * @brief Create remote instances
   *
   * @param id Internal identifier of the instance
   * @param name Class name
   * @param parameters Constructor parameters.
   */
  void remote_make_handle(ObjectId id, const std::string &name,
                          const VariantMap &parameters);

private:
  /**
   * @brief Callback for @function remote_set_parameter
   */
  void set_parameter(ObjectId id, std::string const &name,
                     PackedVariant const &value);

public:
  void notify_set_parameter(const ObjectHandle *o, std::string const &name,
                            Variant const &value) override;

private:
  /**
   * @brief Callback for @function remote_call_method
   */
  void call_method(ObjectId id, std::string const &name,
                   PackedMap const &arguments);

public:
  void nofity_call_method(const ObjectHandle *o, std::string const &name,
                          VariantMap const &arguments) override;

private:
  /**
   * @brief Callback for @function remote_delete_handle
   */
  void delete_handle(ObjectId id) { m_local_objects.erase(id); }

public:
  /**
   * @brief Delete remote instances
   *
   * @param o Internal identified of the instance
   */
  void nofity_delete_handle(const ObjectHandle *o) override;

  /**
   * @brief Get a new reference counted instance of a script interface by
   * name.
   *
   */
  std::shared_ptr<ObjectHandle>
  make_shared(std::string const &name, const VariantMap &parameters) override;
};
} // namespace ScriptInterface

#endif
