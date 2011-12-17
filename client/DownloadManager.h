/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H
#define DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H

#include "forward.h"

#include "DownloadManagerListener.h"
#include "UserConnectionListener.h"
#include "QueueItem.h"
#include "TimerManager.h"
#include "Singleton.h"
#include "MerkleTree.h"
#include "Speaker.h"

namespace dcpp {

/**
 * Singleton. Use its listener interface to update the download list
 * in the user interface.
 */
class DownloadManager : public Speaker<DownloadManagerListener>, 
	private UserConnectionListener, private TimerManagerListener, 
	public Singleton<DownloadManager>
{
public:

	/** @internal */
	void addConnection(UserConnectionPtr conn);
	bool checkIdle(const UserPtr& user, bool smallSlot, bool reportOnly = false);
	void sendBundleMode(BundlePtr aBundle, bool singleUser);
	void setTarget(const string& oldTarget, const string& newTarget);
	void changeBundle(BundlePtr sourceBundle, BundlePtr targetBundle, const string& path);
	void sendBundleFinished(BundlePtr aBundle);
	void sendSizeNameUpdate(BundlePtr aBundle);
	BundlePtr findRunningBundle(const string& bundleToken);

	/** @internal */
	void abortDownload(const string& aTarget, const UserPtr& aUser = NULL);
	void disconnectBundle(BundlePtr aBundle, const UserPtr& aUser = NULL);

	/** @return Running average download speed in Bytes/s */
	int64_t getRunningAverage();

	/** @return Number of downloads. */ 
	size_t getDownloadCount() {
		Lock l(cs);
		return downloads.size();
	}

	bool startDownload(QueueItem::Priority prio, const UserPtr& user, const string& aToken, bool mcn=false);
	
	void updateBundles(BundleList& bundles);

private:
	
	CriticalSection cs;
	DownloadList downloads;
	Bundle::BundleTokenMap runningBundles;
	UserConnectionList idlers;

	void removeRunningUser(UserConnection* aSource, bool sendRemoved=false);
	void removeConnection(UserConnectionPtr aConn);
	void removeDownload(Download* aDown);
	void fileNotAvailable(UserConnection* aSource);
	void noSlots(UserConnection* aSource, string param = Util::emptyString);
	
	int64_t getResumePos(const string& file, const TigerTree& tt, int64_t startPos);

	void failDownload(UserConnection* aSource, const string& reason);

	friend class Singleton<DownloadManager>;

	DownloadManager();
	~DownloadManager();

	typedef unordered_set<CID> CIDList;
	void checkDownloads(UserConnection* aConn);
	void startData(UserConnection* aSource, int64_t start, int64_t newSize, bool z);
	void startBundle(UserConnection* aSource, BundlePtr aBundle);
	bool sendBundle(UserConnection* aSource, BundlePtr aBundle, bool updateOnly);
	void sendBundleUBN(HintedUser& user, const string& speed, const double percent, const string& bundleToken);
	typedef unordered_map<string, BundlePtr> tokenMap;
	tokenMap tokens;

	string formatDownloaded(int64_t aBytes);
	void endData(UserConnection* aSource);

	void onFailed(UserConnection* aSource, const string& aError);

	// UserConnectionListener
	void on(Data, UserConnection*, const uint8_t*, size_t) noexcept;
	void on(Failed, UserConnection* aSource, const string& aError) noexcept { onFailed(aSource, aError); }
	void on(ProtocolError, UserConnection* aSource, const string& aError) noexcept { onFailed(aSource, aError); }
	void on(MaxedOut, UserConnection*, string param = Util::emptyString) noexcept;
	void on(FileNotAvailable, UserConnection*) noexcept;
	void on(Updated, UserConnection*) noexcept;
		
	void on(AdcCommand::SND, UserConnection*, const AdcCommand&) noexcept;
	void on(AdcCommand::STA, UserConnection*, const AdcCommand&) noexcept;

	// TimerManagerListener
	void on(TimerManagerListener::Second, uint64_t aTick) noexcept;

	typedef pair< string, int64_t > StringIntPair;

};

} // namespace dcpp

#endif // !defined(DOWNLOAD_MANAGER_H)

/**
 * @file
 * $Id: DownloadManager.h 568 2011-07-24 18:28:43Z bigmuscle $
 */
