#include "driver.h"

typedef struct TWI_Event_t {
} TWI_Event_t;

terr_t TWI_Event_create (TW_Event_handler_t TWI_UNUSED evt_cb,
						 void TWI_UNUSED *evt_data,
						 TW_Event_args_t TWI_UNUSED args,
						 void TWI_UNUSED *dispatcher_obj,
						 TW_Handle_t TWI_UNUSED *hevt) {
	TWI_Event_t *ep = (TWI_Event_t *)TWI_Malloc (sizeof (TWI_Event_t));

	TWI_Active_evt_driver->Event_create (evt_cb, evt_data, args, )

		return TW_ERR_NOT_SUPPORTED;
}  // Create a new event
terr_t TWI_Event_free (TW_Handle_t TWI_UNUSED hevt) { return TW_ERR_NOT_SUPPORTED; }
terr_t TWI_Event_commit (TW_Handle_t TWI_UNUSED engine, TW_Handle_t TWI_UNUSED hevt) {
	return TW_ERR_NOT_SUPPORTED;
}  // Commit event, start watching
terr_t TWI_Event_retract (TW_Handle_t TWI_UNUSED hevt) {
	return TW_ERR_NOT_SUPPORTED;
}  // Stop watching