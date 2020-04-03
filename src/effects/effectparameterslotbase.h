#ifndef EFFECTPARAMETERSLOTBASE_H
#define EFFECTPARAMETERSLOTBASE_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "control/controlobject.h"
#include "effects/effectmanifest.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;
class EffectParameter;
class EffectSlot;


// EffectParameterSlotBase is a wrapper around the parameterX ControlObject
// that loaded with an EffectParameter into itself by the EffectSlot.
class EffectParameterSlotBase : public QObject {
    Q_OBJECT
  public:
    EffectParameterSlotBase(const QString& group, const unsigned int iParameterSlotNumber,
            const EffectManifestParameter::ParameterType parameterType);

    virtual ~EffectParameterSlotBase();

    virtual void loadParameter(EffectParameterPointer pEffectParameter) = 0;

    // Clear the currently loaded effect
    virtual void clear() = 0;

    virtual void syncSofttakeover();

    virtual void onEffectMetaParameterChanged(double parameter, bool force=false);

    QString name() const;
    QString shortName() const;
    QString description() const;
    EffectManifestParameter::ParameterType parameterType() const;
    EffectManifestParameterPointer getManifest();
    inline bool isLoaded() const {
        return m_pManifestParameter != nullptr;
    }

  signals:
    // Signal that indicates that the EffectParameterSlotBase has been updated.
    void updated();

  public slots:
    // Solely for handling control changes
    void slotValueChanged(double v);
    virtual void slotParameterValueChanged(double value) = 0;

  protected:
    const unsigned int m_iParameterSlotNumber;
    QString m_group;
    EffectSlot* m_pEffectSlot;
    EffectParameterPointer m_pEffectParameter;
    EffectManifestParameterPointer m_pManifestParameter;
    EffectManifestParameter::ParameterType m_parameterType;

    // Controls exposed to the rest of Mixxx
    ControlObject* m_pControlLoaded;
    ControlObject* m_pControlType;
    double m_dChainParameter;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlotBase);
};

#endif /* EFFECTPARAMETERSLOTBASE_H */
