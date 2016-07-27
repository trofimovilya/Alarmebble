package ru.ilyatrofimov.alarmebble.reciever

import android.content.Context
import android.content.Intent
import android.preference.PreferenceManager
import android.provider.AlarmClock
import com.getpebble.android.kit.PebbleKit
import com.getpebble.android.kit.util.PebbleDictionary
import ru.ilyatrofimov.alarmebble.R
import java.util.*

class AlarmebbleReceiver : PebbleKit.PebbleDataReceiver(APP_UUID) {
    private val STATUS_OK = 0
    private val STATUS_ERROR = -2

    override fun receiveData(context: Context, transactionId: Int, data: PebbleDictionary) {
        PebbleKit.sendAckToPebble(context, transactionId)
        val response = PebbleDictionary()

        // Get current alarm app
        val settings = PreferenceManager.getDefaultSharedPreferences(context);
        val processName = settings.getString(context.getString(R.string.key_process), "")
        val activityName = settings.getString(context.getString(R.string.key_activity), "")

        // Try to start set alarm intent and prepare response for pebble
        try {
            with (Intent(AlarmClock.ACTION_SET_ALARM)) {
                setClassName(processName, activityName)
                putExtra(AlarmClock.EXTRA_HOUR, data.getInteger(0).toInt())
                putExtra(AlarmClock.EXTRA_MINUTES, data.getInteger(1).toInt())
                putExtra(AlarmClock.EXTRA_SKIP_UI, true)
                flags = Intent.FLAG_ACTIVITY_NEW_TASK

                if (resolveActivity(context.packageManager) != null) {
                    context.startActivity(this)
                    response.addInt32(0, STATUS_OK) // OK
                } else {
                    response.addInt32(0, STATUS_ERROR) // Error
                }
            }
        } catch (e: Exception) {
            response.addInt32(0, STATUS_ERROR) // Error
        }

        // Send response to Pebble
        PebbleKit.sendDataToPebbleWithTransactionId(context, APP_UUID, response, transactionId)
    }

    /**
     * Not secret Pebble APP_UUID
     */
    companion object {
        private val APP_UUID = UUID.fromString("e44a8ca9-2d35-4d55-b9f2-cafd12788dbf")
    }
}
