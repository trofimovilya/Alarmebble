package ru.ilyatrofimov.alarmebble.ui

import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.AlarmClock
import android.support.v7.app.AlertDialog
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.LinearLayoutManager
import android.view.Menu
import android.view.MenuItem
import android.view.View
import kotlinx.android.synthetic.main.activity_main.*
import ru.ilyatrofimov.alarmebble.R

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setSupportActionBar(toolbar)
    }

    override fun onResume() {
        super.onResume()

        val setAlarmIntent = Intent(AlarmClock.ACTION_SET_ALARM)
        val pkgAppsList = packageManager.queryIntentActivities(setAlarmIntent
                , PackageManager.MATCH_DEFAULT_ONLY)

        if (pkgAppsList.isNotEmpty()) {
            val listAdapter = AppsAdapter(this)
            listAdapter.setAppsList(pkgAppsList)
            placeholder.visibility = View.GONE
            list.visibility = View.VISIBLE
            list.adapter = listAdapter
            list.layoutManager = LinearLayoutManager(this)
            list.addItemDecoration(DividerItemDecoration(this))
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_main, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        AlertDialog.Builder(this)
                .setTitle(getString(R.string.caution))
                .setMessage(getString(R.string.caution_text))
                .setPositiveButton(android.R.string.yes, { dialogInterface, i ->
                    packageManager.setComponentEnabledSetting(componentName, PackageManager
                            .COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP)
                })
                .setNegativeButton(android.R.string.cancel, { dialogInterface, i ->
                    dialogInterface.cancel()
                }).show()

        return true
    }
}
