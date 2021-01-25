////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_WMS_OVERLAYS_PLUGIN_HPP
#define CSP_WMS_OVERLAYS_PLUGIN_HPP

#include "utils.hpp"

#include "../../../src/cs-core/PluginBase.hpp"
#include "../../../src/cs-utils/DefaultProperty.hpp"

#include <chrono>
#include <map>
#include <string>

namespace csp::wmsoverlays {

class TextureOverlayRenderer;
class WebMapService;
class WebMapLayer;

/// This plugin provides the rendering of planets as spheres with a texture and an additional WMS
/// based texture. Despite its name it can also render moons :P. It can be configured via the
/// applications config file. See README.md for details.
class Plugin : public cs::core::PluginBase {
 public:
  /// The startup settings of the plugin.
  struct Settings {
    /// Specifies whether to interpolate textures between timesteps (does not work when pre-fetch is
    /// inactive).
    cs::utils::DefaultProperty<bool> mEnableInterpolation{true};

    /// Specifies whether to display timespan.
    /// Needs to be specified true in config for a data set which enables it in order to be used.
    cs::utils::DefaultProperty<bool> mEnableTimespan{false};

    /// Specifies whether to autmatically update the overlay bounds when the observer stopped moving
    /// for a certain amount of time.
    cs::utils::DefaultProperty<bool> mEnableAutomaticBoundsUpdate{false};

    /// Path to the map cache folder, can be absolute or relative to the cosmoscout executable.
    cs::utils::DefaultProperty<std::string> mMapCache{"texture-cache"};

    /// Path to the wms capability cache folder, can be absolute or relative to the cosmoscout
    /// executable.
    cs::utils::DefaultProperty<std::string> mCapabilityCache{"wms-capability-cache"};

    /// The amount of textures that gets pre-fetched in every time direction.
    cs::utils::DefaultProperty<int> mPrefetchCount{0};

    /// The size of the requested map textures along the longer axis. Some wms layers may only be
    /// available in certain sizes, those won't be influenced by this setting.
    cs::utils::DefaultProperty<int> mMaxTextureSize{1024};

    /// The startup settings for a planet.
    struct Body {
      cs::utils::DefaultProperty<std::string> mActiveServer{
          "None"}; ///< The name of the currently active WMS server.
      cs::utils::DefaultProperty<std::string> mActiveLayer{
          "None"}; ///< The name of the currently active WMS layer.
      cs::utils::DefaultProperty<std::string> mActiveStyle{
          ""};                       ///< The name of the style for the currently active WMS layer.
      std::vector<std::string> mWms; ///<	URLs of WMS servers.
    };

    std::map<std::string, Body> mBodies; ///< A list of bodies with their anchor names.
  };

  void init() override;
  void deInit() override;

  void update() override;

 private:
  void onLoad();

  Settings::Body& getBodySettings(std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay) const;

  void setWMSServer(
      std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay, std::string const& name);
  void resetWMSServer(std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay);
  void setWMSLayer(
      std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay, std::string const& name);
  void resetWMSLayer(std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay);
  void setWMSStyle(
      std::shared_ptr<TextureOverlayRenderer> const& wmsOverlay, std::string const& name);
  void resetWMSStyle(std::shared_ptr<TextureOverlayRenderer > const& wmsOverlay);

  void goToBounds(Bounds bounds);

  std::shared_ptr<Settings> mPluginSettings = std::make_shared<Settings>();
  std::map<std::string, std::shared_ptr<TextureOverlayRenderer>> mWMSOverlays;
  std::map<std::string, std::vector<WebMapService>>              mWms;

  std::shared_ptr<TextureOverlayRenderer>             mActiveOverlay;
  std::map<std::string, std::optional<WebMapService>> mActiveServers;
  std::map<std::string, std::optional<WebMapLayer>>   mActiveLayers;

  bool mNoMovement;                ///< True when the observer is not moving.
  bool mNoMovementRequestedUpdate; ///< True when the active overlay was requested to update its
                                   ///< bounds because the observer is not moving.
  std::chrono::time_point<std::chrono::steady_clock>
      mNoMovementSince; ///< Time at which the observer stopped moving.

  int mActiveBodyConnection    = -1;
  int mObserverSpeedConnection = -1;
  int mOnLoadConnection        = -1;
  int mOnSaveConnection        = -1;
  int mBoundsConnection        = -1;
};

} // namespace csp::wmsoverlays

#endif // CSP_WMS_OVERLAYS_PLUGIN_HPP
