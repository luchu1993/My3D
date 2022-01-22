//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "Core/Object.h"


namespace My3D
{
    class MY3D_API FileSystem : public Object
    {
        MY3D_OBJECT(FileSystem, Object);
    public:
        /// Construct
        explicit FileSystem(Context* context);
        /// Destruct
        ~FileSystem() override;
        /// Set whether to execute engine console commands as OS-specific system command.
        void SetExecuteConsoleCommands(bool enable);

    private:
        /// Handle begin frame event to check for completed async executions.
        void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
        /// Handle a console command event.
        void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

        /// Flag for executing engine console commands as OS-specific system command. Default to true.
        bool executeConsoleCommands_{};
    };
}
