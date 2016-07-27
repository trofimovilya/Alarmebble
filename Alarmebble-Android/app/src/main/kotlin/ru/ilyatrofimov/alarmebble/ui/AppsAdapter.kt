package ru.ilyatrofimov.alarmebble.ui

import android.content.Context
import android.content.SharedPreferences
import android.content.pm.ResolveInfo
import android.preference.PreferenceManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import kotlinx.android.synthetic.main.list_item_app.view.*
import ru.ilyatrofimov.alarmebble.R
import java.util.*

class AppsAdapter(private val context: Context) : RecyclerView.Adapter<AppsAdapter.ViewHolder>() {
    private var appsList: List<ResolveInfo>
    private var checkedPosition: Int
    private var selectedActivity: String
    private val settings: SharedPreferences

    init {
        checkedPosition = -1
        appsList = ArrayList()
        settings = PreferenceManager.getDefaultSharedPreferences(context)
        selectedActivity = settings.getString(context.getString(R.string.key_activity), "")
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int) = ViewHolder(LayoutInflater
            .from(parent.context).inflate(R.layout.list_item_app, parent, false))

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val app = appsList[position]

        if (selectedActivity.equals(app.activityInfo.name)) {
            checkedPosition = position
        }

        with (holder) {
            radioButton.isChecked = position == checkedPosition
            icon.setImageDrawable(app.loadIcon(context.packageManager))
            appName.text = app.loadLabel(context.packageManager)
            activityName.text = app.activityInfo.name
            activityName.isSelected = radioButton.isChecked

            itemView.setOnClickListener {
                if (!radioButton.isChecked && checkedPosition != position) {
                    settings.edit()
                            .putString(context.getString(R.string.key_process)
                                    , app.activityInfo.processName)
                            .putString(context.getString(R.string.key_activity)
                                    , app.activityInfo.name)
                            .apply()

                    selectedActivity = app.activityInfo.name
                    radioButton.isChecked = true
                    activityName.isSelected = true
                    notifyItemChanged(checkedPosition)
                    checkedPosition = position
                }
            }
        }
    }

    override fun getItemCount() = appsList.size

    fun setAppsList(appsList: List<ResolveInfo>) {
        this.appsList = appsList
        notifyDataSetChanged()
    }

    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val icon = view.icon
        val appName = view.text_app_name
        val activityName = view.text_activity_name
        val radioButton = view.radio_button
    }
}
